// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "ast.h"
#include "branch.h"
#include "compilation.h"
#include "function.h"
#include "parser.h"
#include "pointer_visitor.h"
#include "runtime.h"
#include "tokenizer.h"

namespace circa {

Term* compile_statement(Branch* branch, std::string const& input)
{
    assert(branch != NULL);

    TokenStream tokens(input);

    ast::Statement* statementAst = parser::statement(tokens);
    assert(statementAst != NULL);

    Term* result = statementAst->createTerm(*branch);
    assert(result != NULL);

    remove_compilation_attrs(*branch);

    delete statementAst;

    return result;
}

Term* eval_statement(Branch* branch, std::string const& input)
{
    Term* term = compile_statement(branch, input);
    evaluate_term(term);
    return term;
}

namespace parser {

std::string possibleWhitespace(TokenStream& tokens);
std::string possibleNewline(TokenStream& tokens);
std::string possibleWhitespaceNewline(TokenStream& tokens);

ast::StatementList* statementList(TokenStream& tokens)
{
    ast::StatementList* statementList = new ast::StatementList();

    while (!tokens.finished()) {

        std::string leadingWhitespace = possibleWhitespace(tokens);

        if (tokens.nextIs(tokenizer::END) || tokens.nextIs(tokenizer::ELSE)) {
            break;
        }

        ast::Statement *statemnt = statement(tokens);

        statemnt->precedingWhitespace = leadingWhitespace + statemnt->precedingWhitespace;

        statementList->push(statemnt);
    }

    return statementList;
}

ast::Statement* statement(TokenStream& tokens)
{
    std::string precedingWhitespace = possibleWhitespace(tokens);

    ast::Statement* result = NULL;

    // Check for comment line
    if (tokens.nextIs(tokenizer::DOUBLE_MINUS)) {
        std::stringstream commentText;
        tokens.consume(tokenizer::DOUBLE_MINUS);

        // Throw away the rest of this line
        while (!tokens.finished()) {
            if (tokens.nextIs(tokenizer::NEWLINE)) {
                tokens.consume(tokenizer::NEWLINE);
                break;
            }

            commentText << tokens.next().text;
            tokens.consume();
        }

        result = new ast::CommentStatement(commentText.str());
    }

    // Check for blank line
    else if (tokens.nextIs(tokenizer::NEWLINE) || tokens.finished()) {
        if (tokens.nextIs(tokenizer::NEWLINE))
            tokens.consume(tokenizer::NEWLINE);
        result = new ast::CommentStatement("");
    }

    // Check for keywords
    else if (tokens.nextIs(tokenizer::FUNCTION)) {
        result = functionDecl(tokens);
    }

    else if (tokens.nextIs(tokenizer::TYPE)) {
        result = typeDecl(tokens);
    }

    else if (tokens.nextIs(tokenizer::IF)) {
        result = ifStatement(tokens);
    }

    else if (tokens.nextIs(tokenizer::STATE)) {
        result = statefulValueDeclaration(tokens);
    }

    else {
        result = expressionStatement(tokens);
    }

    result->precedingWhitespace = precedingWhitespace;

    return result;
}

ast::ExpressionStatement* expressionStatement(TokenStream& tokens)
{
    ast::ExpressionStatement* statement = new ast::ExpressionStatement();

    bool isNameBinding = tokens.nextIs(tokenizer::IDENTIFIER)
        && (tokens.nextIs(tokenizer::EQUALS, 1)
                || (tokens.nextIs(tokenizer::WHITESPACE,1)
                    && (tokens.nextIs(tokenizer::EQUALS,2))));

    // check for name binding
    if (isNameBinding) {
        statement->nameBinding = tokens.consume(tokenizer::IDENTIFIER);
        statement->preEqualsWhitepace = possibleWhitespace(tokens);
        tokens.consume(tokenizer::EQUALS);
        statement->postEqualsWhitespace = possibleWhitespace(tokens);
    }

    // check for return statement
    else if (tokens.nextIs(tokenizer::IDENTIFIER) && tokens.next().text == "return") {
        tokens.consume(tokenizer::IDENTIFIER);
        statement->nameBinding = OUTPUT_PLACEHOLDER_NAME;
        statement->preEqualsWhitepace = "";
        statement->postEqualsWhitespace = possibleWhitespace(tokens);
    }

    statement->expression = infixExpression(tokens);

    if (tokens.nextIs(tokenizer::NEWLINE))
        tokens.consume(tokenizer::NEWLINE);

    return statement;
}

int getInfixPrecedence(int match)
{
    switch(match) {
        case tokenizer::DOT:
            return 8;
        case tokenizer::STAR:
        case tokenizer::SLASH:
            return 7;
        case tokenizer::PLUS:
        case tokenizer::MINUS:
            return 6;
        case tokenizer::LTHAN:
        case tokenizer::LTHANEQ:
        case tokenizer::GTHAN:
        case tokenizer::GTHANEQ:
        case tokenizer::DOUBLE_EQUALS:
        case tokenizer::NOT_EQUALS:
            return 5;
        case tokenizer::DOUBLE_AMPERSAND:
        case tokenizer::DOUBLE_VERTICAL_BAR:
            return 4;
        case tokenizer::EQUALS:
        case tokenizer::PLUS_EQUALS:
        case tokenizer::MINUS_EQUALS:
        case tokenizer::STAR_EQUALS:
        case tokenizer::SLASH_EQUALS:
            return 2;
        case tokenizer::RIGHT_ARROW:
            return 1;
        case tokenizer::COLON_EQUALS:
            return 0;
        default:
            return -1;
    }
}

#define HIGHEST_INFIX_PRECEDENCE 8

ast::Expression* infixExpression(TokenStream& tokens,
        int precedence)
{
    if (precedence > HIGHEST_INFIX_PRECEDENCE)
        return atom(tokens);

    ast::Expression* leftExpr = infixExpression(tokens, precedence+1);

    possibleWhitespace(tokens);

    while (!tokens.finished() && getInfixPrecedence(tokens.next().match) == precedence) {

        std::string operatorStr = tokens.consume();

        possibleWhitespace(tokens);

        ast::Expression* rightExpr = infixExpression(tokens, precedence+1);

        leftExpr = new ast::Infix(operatorStr, leftExpr, rightExpr);
    }

    return leftExpr;
}

void post_literal(TokenStream& tokens, ast::Literal* literal)
{
    if (tokens.nextIs(tokenizer::QUESTION)) {
        tokens.consume(tokenizer::QUESTION);
        literal->hasQuestionMark = true;
    }
}

ast::Literal* literal_float(TokenStream& tokens)
{
    ast::Literal* lit = new ast::LiteralFloat(tokens.consume(tokenizer::FLOAT));
    post_literal(tokens, lit);
    return lit;
}

ast::Literal* literal_integer(TokenStream& tokens)
{
    ast::Literal* lit = new ast::LiteralInteger(tokens.consume(tokenizer::INTEGER));
    post_literal(tokens, lit);
    return lit;
}

ast::Literal* literal_hex(TokenStream& tokens)
{
    ast::Literal* lit = new ast::LiteralInteger(tokens.consume(tokenizer::HEX_INTEGER));
    post_literal(tokens, lit);
    return lit;
}

ast::Literal* literal_string(TokenStream& tokens)
{
    ast::Literal* lit = new ast::LiteralString(tokens.consume());
    post_literal(tokens, lit);
    return lit;
}

ast::Expression* atom(TokenStream& tokens)
{
    // function call?
    if (tokens.nextIs(tokenizer::IDENTIFIER) && tokens.nextIs(tokenizer::LPAREN, 1))
        return functionCall(tokens);

    // literal string?
    if (tokens.nextIs(tokenizer::STRING))
        return literal_string(tokens);

    // literal float?
    if (tokens.nextIs(tokenizer::FLOAT))
        return literal_float(tokens);

    // literal integer?
    if (tokens.nextIs(tokenizer::INTEGER))
        return literal_integer(tokens);

    // literal hex?
    if (tokens.nextIs(tokenizer::HEX_INTEGER))
        return literal_hex(tokens);

    // rebind operator?
    bool hasRebindOperator = false;

    if (tokens.nextIs(tokenizer::AMPERSAND)) {
        hasRebindOperator = true;
        tokens.consume(tokenizer::AMPERSAND);
    }

    // identifier?
    if (tokens.nextIs(tokenizer::IDENTIFIER)) {
        ast::Identifier* id = new ast::Identifier(tokens.consume(tokenizer::IDENTIFIER));
        id->hasRebindOperator = hasRebindOperator;
        return id;
    } else if (hasRebindOperator) {
        syntax_error("@ operator only allowed before an identifier");
    }

    // parenthesized expression?
    if (tokens.nextIs(tokenizer::LPAREN)) {
        tokens.consume(tokenizer::LPAREN);
        ast::Expression* expr = infixExpression(tokens);
        tokens.consume(tokenizer::RPAREN);
        return expr;
    }

    syntax_error("Unrecognized expression", &tokens.next());

    return NULL; // unreachable
}

ast::FunctionCall* functionCall(TokenStream& tokens)
{
    std::string functionName = tokens.consume(tokenizer::IDENTIFIER);
    tokens.consume(tokenizer::LPAREN);

    std::auto_ptr<ast::FunctionCall> functionCall(new ast::FunctionCall(functionName));

    while (!tokens.nextIs(tokenizer::RPAREN))
    {
        std::string preWhitespace = possibleWhitespace(tokens);
        std::auto_ptr<ast::Expression> expression(infixExpression(tokens));
        std::string postWhitespace = possibleWhitespace(tokens);

        functionCall->addArgument(expression.release(), preWhitespace, postWhitespace);

        if (!tokens.nextIs(tokenizer::RPAREN))
            tokens.consume(tokenizer::COMMA);
    }

    tokens.consume(tokenizer::RPAREN);
    
    return functionCall.release();
}

ast::FunctionHeader* functionHeader(TokenStream& tokens)
{
    std::auto_ptr<ast::FunctionHeader> header(new ast::FunctionHeader());

    if (tokens.nextIs(tokenizer::FUNCTION))
        tokens.consume(tokenizer::FUNCTION);
    possibleWhitespace(tokens);

    header->functionName = tokens.consume(tokenizer::IDENTIFIER);
    possibleWhitespace(tokens);

    tokens.consume(tokenizer::LPAREN);

    while (!tokens.nextIs(tokenizer::RPAREN))
    {
        std::string preWhitespace = possibleWhitespace(tokens);
        std::string type = tokens.consume(tokenizer::IDENTIFIER);
        std::string innerWhitespace = possibleWhitespace(tokens);

        std::string name, postWhitespace;
        if (tokens.nextIs(tokenizer::COMMA) || tokens.nextIs(tokenizer::RPAREN)) {
            name = "";
            postWhitespace = "";
        } else {
            name = tokens.consume(tokenizer::IDENTIFIER);
            postWhitespace = possibleWhitespace(tokens);
        }

        header->addArgument(type, name);

        if (!tokens.nextIs(tokenizer::RPAREN))
            tokens.consume(tokenizer::COMMA);
    }

    tokens.consume(tokenizer::RPAREN);

    possibleWhitespace(tokens);

    if (tokens.nextIs(tokenizer::RIGHT_ARROW)) {
        tokens.consume(tokenizer::RIGHT_ARROW);
        possibleWhitespace(tokens);
        header->outputType = tokens.consume(tokenizer::IDENTIFIER);
        possibleWhitespace(tokens);
    }

    return header.release();
}

ast::FunctionDecl* functionDecl(TokenStream& tokens)
{
    std::auto_ptr<ast::FunctionDecl> decl(new ast::FunctionDecl());

    decl->header = functionHeader(tokens);

    possibleNewline(tokens);

    /*tokens.consume(tokenizer::LBRACE);

    possibleWhitespaceNewline(tokens);*/

    decl->statements = statementList(tokens);

    possibleWhitespaceNewline(tokens);

    tokens.consume(tokenizer::END);

    possibleWhitespaceNewline(tokens);
    
    return decl.release();
}

ast::TypeDecl* typeDecl(TokenStream& tokens)
{
    std::auto_ptr<ast::TypeDecl> decl(new ast::TypeDecl());

    tokens.consume(tokenizer::TYPE);

    possibleWhitespace(tokens);

    decl->name = tokens.consume(tokenizer::IDENTIFIER);

    possibleWhitespaceNewline(tokens);

    tokens.consume(tokenizer::LBRACE);

    possibleWhitespaceNewline(tokens);

    while (true) {

        possibleWhitespace(tokens);

        if (tokens.nextIs(tokenizer::RBRACE)) {
            tokens.consume(tokenizer::RBRACE);
            break;
        }

        std::string fieldType = tokens.consume(tokenizer::IDENTIFIER);
        possibleWhitespace(tokens);
        std::string fieldName = tokens.consume(tokenizer::IDENTIFIER);
        possibleWhitespace(tokens);

        decl->addMember(fieldType, fieldName);

        tokens.consume(tokenizer::NEWLINE);
    }

    return decl.release();
}

ast::IfStatement* ifStatement(TokenStream& tokens)
{
    tokens.consume(tokenizer::IF);
    possibleWhitespace(tokens);

    std::auto_ptr<ast::IfStatement> result(new ast::IfStatement());

    result->condition = infixExpression(tokens);

    possibleWhitespaceNewline(tokens);

    result->positiveBranch = statementList(tokens);

    if (tokens.nextIs(tokenizer::ELSE)) {
        tokens.consume(tokenizer::ELSE);
        possibleWhitespaceNewline(tokens);

        result->negativeBranch = statementList(tokens);
    }

    tokens.consume(tokenizer::END);
    possibleWhitespaceNewline(tokens);

    return result.release();
}

ast::StatefulValueDeclaration* statefulValueDeclaration(TokenStream& tokens)
{
    tokens.consume(tokenizer::STATE);

    possibleWhitespace(tokens);

    std::string type = tokens.consume(tokenizer::IDENTIFIER);
    possibleWhitespace(tokens);
    std::string name = tokens.consume(tokenizer::IDENTIFIER);

    possibleWhitespace(tokens);

    ast::Expression *initialValue = NULL;

    if (tokens.nextIs(tokenizer::EQUALS)) {
        tokens.consume(tokenizer::EQUALS);
        possibleWhitespace(tokens);
        initialValue = infixExpression(tokens);
    }

    possibleWhitespaceNewline(tokens);

    ast::StatefulValueDeclaration *result = new ast::StatefulValueDeclaration();
    result->type = type;
    result->name = name;
    result->initialValue = initialValue;
    return result;
}

std::string possibleWhitespace(TokenStream& tokens)
{
    if (tokens.nextIs(tokenizer::WHITESPACE))
        return tokens.consume(tokenizer::WHITESPACE);
    else
        return "";
}

std::string possibleNewline(TokenStream& tokens)
{
    if (tokens.nextIs(tokenizer::NEWLINE))
        return tokens.consume(tokenizer::NEWLINE);
    else
        return "";
}

std::string possibleWhitespaceNewline(TokenStream& tokens)
{
    std::stringstream output;

    while (tokens.nextIs(tokenizer::NEWLINE) || tokens.nextIs(tokenizer::WHITESPACE))
        output << tokens.consume();

    return output.str();
}

void syntax_error(std::string const& message, tokenizer::TokenInstance const* location)
{
    std::stringstream out;
    out << "Syntax error: " << message;

    if (location != NULL) {
        out << ", at line " << location->line << ", char " << location->character;
    }

    throw std::runtime_error(out.str());
}

} // namespace parser
} // namespace circa
