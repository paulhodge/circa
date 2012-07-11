// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "CircaBindings.h"
#include "RenderEntity.h"
#include "RenderTarget.h"
#include "TextTexture.h"
#include "TextVbo.h"
#include "ShaderUtils.h"

const bool CHECK_GL_ERROR = true;

void TextVbo_Update(GLuint vbo, TextTexture* texture, float posX, float posY);
void TextVbo_Render(GLuint vbo, Program* program, TextTexture* texture, Color color);
void Rect_Update(GLuint vbo, float x1, float y1, float x2, float y2);
void Rect_Render(GLuint vbo, Program* program, Color color);

RenderTarget::RenderTarget()
  : viewportWidth(0),
    viewportHeight(0)
{
    circa_set_map(&textRenderCache);
    circa_set_list(&incomingCommands, 0);

    name_rect = circa_to_name("rect");
    name_textSprite = circa_to_name("textSprite");
    name_AlignHCenter = circa_to_name("AlignHCenter");
    name_AlignVCenter = circa_to_name("AlignVCenter");
}

void
RenderTarget::setup(ResourceManager* resourceManager)
{
    load_shaders(resourceManager, "assets/shaders/Text", &textProgram);
    load_shaders(resourceManager, "assets/shaders/Geom", &geomProgram);
}

void
RenderTarget::sendCommand(caValue* command)
{
    circa_copy(command, circa_append(&incomingCommands));
}

caValue*
RenderTarget::getTextRender(caValue* args)
{
    caValue* value = circa_map_insert(&textRenderCache, args);

    if (circa_is_null(value)) {
        // Value doesn't exist in cache.
        TextTexture* texture = TextTexture::create(this);

        texture->setText(circa_index(args, 0));
        FontRef* font = (FontRef*) circa_object(circa_index(args, 1));
        texture->setFont(font->font_id);
        texture->update();

        circa_set_list(value, 2);
        circa_set_pointer(circa_index(value, 0), texture);
        circa_set_vec2(circa_index(value, 1), texture->width(), texture->height());
    }

    return value;
}

void
RenderTarget::appendEntity(RenderEntity* command)
{
    entities.push_back(command);
}

void
RenderTarget::setViewportSize(int w, int h)
{
    viewportWidth = w;
    viewportHeight = h;
    modelViewProjectionMatrix = glm::ortho(0.0, double(w), double(h), 0.0, -1.0, 100.0);
}

void
RenderTarget::switchProgram(Program* program)
{
    if (currentProgram == program)
        return;

    currentProgram = program;
    glUseProgram(program->program);

    glUniformMatrix4fv(program->uniforms.modelViewProjectionMatrix,
            1, GL_FALSE, glm::value_ptr(modelViewProjectionMatrix));
}

void
RenderTarget::render()
{
    check_gl_error();

    // Run incoming commands
    for (size_t commandIndex=0; commandIndex < circa_count(&incomingCommands); commandIndex++) {
        caValue* command = circa_index(&incomingCommands, commandIndex);
        caName commandName = circa_name(circa_index(command, 0));

        if (commandName == name_textSprite) {
            switchProgram(&textProgram);

            GLuint vbo;
            TextTexture* texture = (TextTexture*) circa_get_pointer(circa_index(command, 1));

            caValue* position = circa_index(command, 2);
            caValue* color = circa_index(command, 3);
            caValue* args = circa_index(command, 4);

            float posX, posY;
            circa_vec2(position, &posX, &posY);

            for (int i=0; i < circa_count(args); i++) {
                caName tag = circa_name(circa_index(args, i));
                if (tag == name_AlignHCenter) {
                    posX -= texture->width() / 2;
                } else if (tag == name_AlignVCenter) {
                    posY += texture->height() / 2;
                }
            }

            TextVbo_Update(vbo, texture, posX, posY);
            TextVbo_Render(vbo, currentProgram, texture, unpack_color(color));
            glDeleteBuffers(1, &vbo);

        } else if (commandName == name_rect) {
            switchProgram(&geomProgram);
            GLuint vbo;

            caValue* pos = circa_index(command, 1);
            caValue* color = circa_index(command, 2);
            float x1, y1, x2, y2;
            circa_vec4(pos, &x1, &y1, &x2, &y2);

            Rect_Update(vbo, x1, y1, x2, y2);
            Rect_Render(vbo, currentProgram, unpack_color(color));
            glDeleteBuffers(1, &vbo);

        } else {
            printf("unrecognized command name: %s\n", circa_name_to_string(commandName));
        }
        
        check_gl_error();
    }

    circa_set_list(&incomingCommands, 0);
}

void
RenderTarget::flushDestroyedEntities()
{
    // Cleanup destroyed entities
    int numDestroyed = 0;
    for (size_t i=0; i < entities.size(); i++) {
        RenderEntity* entity = entities[i];
        if (entity->destroyed()) {
            delete entity;
            numDestroyed++;
            continue;
        }

        if (numDestroyed > 0)
            entities[i - numDestroyed] = entity;
    }
    entities.resize(entities.size() - numDestroyed);
}

void check_gl_error()
{
    if (!CHECK_GL_ERROR)
        return;
    
    GLenum err = glGetError();
    if (err == GL_NO_ERROR)
        return;
    
    while (err != GL_NO_ERROR) {
        const char* str;
        switch (err) {
            case GL_INVALID_ENUM: str = "GL_INVALID_ENUM"; break;
            case GL_INVALID_VALUE: str = "GL_INVALID_VALUE"; break;
            case GL_INVALID_OPERATION: str = "GL_INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW: str = "GL_STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW: str = "GL_STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY: str = "GL_OUT_OF_MEMORY"; break;
            default: str = "(enum not found)";
        }
        printf("OpenGL error: %s\n", str);
        err = glGetError();
    }
    assert(false);
    return;
}

void TextVbo_Update(GLuint vbo, TextTexture* textTexture, float posX, float posY)
{
    assert(textTexture != NULL);
    check_gl_error();
    
    // Upload vertices for polygons
    //
    //       Top
    // Left  [0]  [1]
    //       [2]  [3]
    
    int sizeX = textTexture->metrics.bitmapSizeX;
    int sizeY = textTexture->metrics.bitmapSizeY;
    
    // Adjust position so that (X,Y) is at the origin
    float localPosX = posX - textTexture->metrics.originX;
    float localPosY = posY - textTexture->metrics.originY;
    
    GLfloat vertices[] = {
        // 3 floats for position, 2 for tex coord
        localPosX, localPosY, 0,                   0.0, 0.0,
        localPosX + sizeX, localPosY, 0,           1.0, 0.0,
        localPosX, localPosY + sizeY, 0,           0.0, 1.0,
        localPosX + sizeX, localPosY + sizeY, 0,   1.0, 1.0,
    };
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    check_gl_error();
}

void TextVbo_Render(GLuint vbo, Program* program, TextTexture* textTexture, Color color)
{
    check_gl_error();
    const int floatsPerVertex = 5;
    
    GLuint attribVertex = program->attributes.vertex;
    GLuint attribTexCoord = program->attributes.tex_coord;
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    check_gl_error();

    glEnableVertexAttribArray(attribVertex);
    glVertexAttribPointer(attribVertex, 3, GL_FLOAT, GL_FALSE, floatsPerVertex*4, BUFFER_OFFSET(0));
    check_gl_error();
    
    glEnableVertexAttribArray(attribTexCoord);
    glVertexAttribPointer(attribTexCoord, 2, GL_FLOAT, GL_FALSE, floatsPerVertex*4, BUFFER_OFFSET(12));
    check_gl_error();
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textTexture->texid);
    check_gl_error();
    
    glUniform1i(program->uniforms.sampler, 0);
    glUniform4f(program->uniforms.color, color.r, color.g, color.b, color.a);
    check_gl_error();
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // cleanup
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    check_gl_error();
}

void Rect_Update(GLuint vbo, float x1, float y1, float x2, float y2)
{
    GLfloat vertices[] = {
        // 3 floats for position, 2 for tex coord
        x1, y1, 0,          0.0, 0.0,
        x2, y1, 0,          1.0, 0.0,
        x1, y2, 0,          0.0, 1.0,
        x2, y2, 0,          1.0, 1.0,
    };

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Rect_Render(GLuint vbo, Program* program, Color color)
{
    const int floatsPerVertex = 5;

    GLuint attribVertex = program->attributes.vertex;

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glEnableVertexAttribArray(attribVertex);
    glVertexAttribPointer(attribVertex, 3, GL_FLOAT, GL_FALSE, floatsPerVertex*4, BUFFER_OFFSET(0));
    check_gl_error();
    
    glUniform4f(program->uniforms.color, color.r, color.g, color.b, color.a);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // cleanup
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
