// Copyright 2008 Andrew Fischer

#include "tokenizer.h"

namespace circa {
namespace tokenizer {

const char* getMatchText(int match)
{
    switch (match) {
        case LPAREN: return "(";
        case RPAREN: return ")";
        case LBRACE: return "{";
        case RBRACE: return "}";
        case LBRACKET: return "[";
        case RBRACKET: return "]";
        case COMMA: return ",";
        case AMPERSAND: return "@";
        case IDENTIFIER: return "IDENTIFIER";
        case INTEGER: return "INTEGER";
        case FLOAT: return "FLOAT";
        case STRING: return "STRING";
        case QUOTED_IDENTIFIER: return "QUOTED_IDENTIFIER";
        case DOT: return ".";
        case STAR: return "*";
        case QUESTION: return "?";
        case SLASH: return "/";
        case PLUS: return "+";
        case MINUS: return "-";
        case LTHAN: return "<";
        case LTHANEQ: return "<=";
        case GTHAN: return ">";
        case GTHANEQ: return ">=";
        case DOUBLE_EQUALS: return "==";
        case NOT_EQUALS: return "!=";
        case EQUALS: return "=";
        case PLUS_EQUALS: return "+=";
        case MINUS_EQUALS: return "-=";
        case STAR_EQUALS: return "*=";
        case SLASH_EQUALS: return "/=";
        case RIGHT_ARROW: return "->";
        case WHITESPACE: return "WHITESPACE";
        case NEWLINE: return "NEWLINE";
        case UNRECOGNIZED: return "UNRECOGNIZED";
        default: return "NOT FOUND";
    }
}

struct TokenizeContext
{
    std::string const &input;
    int nextIndex;
    int linePosition;
    int charPosition;
    std::vector<TokenInstance> &results;

    TokenizeContext(std::string const &_input, std::vector<TokenInstance> &_results)
        : input(_input),
          nextIndex(0),
          linePosition(1),
          charPosition(1),
          results(_results)
    {
    }

    char next(int lookahead=0) const {
        unsigned int index = nextIndex + lookahead;
        if (index >= input.length())
            return 0;
        return input[index];
    }

    char consume() {
        if (finished())
            return 0;

        char c = next();
        nextIndex++;

        if (c == '\n') {
            this->linePosition++;
            this->charPosition = 1;
        } else
            this->charPosition++;

        return c;
    }

    bool finished() const {
        return nextIndex >= (int) input.length();
    }

    void pushResult(int match, std::string text = "") {
        if (text == "" && match != STRING)
            text = getMatchText(match);

        TokenInstance instance;
        instance.match = match;
        instance.text = text;

        instance.line = this->linePosition;

        // I think this will give us the last character of the word. Will fix.
        instance.character = this->charPosition;

        results.push_back(instance);
    }
};

void top_level_consume_token(TokenizeContext &context);
void consume_identifier(TokenizeContext &context);
void consume_whitespace(TokenizeContext &context);
void consume_number(TokenizeContext &context);
void consume_string_literal(TokenizeContext &context);
void consume_quoted_identifier(TokenizeContext &context);

void tokenize(std::string const &input, TokenList &results)
{
    TokenizeContext context(input, results);

    while (!context.finished()) {
        top_level_consume_token(context);
    }
}

bool is_letter(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool is_number(char c)
{
    return (c >= '0' && c <= '9');
}

bool is_acceptable_inside_identifier(char c)
{
    return is_letter(c) || is_number(c) || c == '_' || c == '-';
}

bool is_whitespace(char c)
{
    return c == ' ' || c == '\t';
}

void top_level_consume_token(TokenizeContext &context)
{
    if (is_letter(context.next())) {
        consume_identifier(context);
        return;
    }

    if (is_whitespace(context.next())) {
        consume_whitespace(context);
        return;
    }

    if (is_number(context.next())) {
        consume_number(context);
        return;
    }

    // Check for specific characters
    switch(context.next()) {
        case '(':
            context.consume();
            context.pushResult(LPAREN, "(");
            return;
        case ')':
            context.consume();
            context.pushResult(RPAREN, ")");
            return;
        case '{':
            context.consume();
            context.pushResult(LBRACE);
            return;
        case '}':
            context.consume();
            context.pushResult(RBRACE);
            return;
        case '[':
            context.consume();
            context.pushResult(LBRACKET);
            return;
        case ']':
            context.consume();
            context.pushResult(RBRACKET);
            return;
        case ',':
            context.consume();
            context.pushResult(COMMA);
            return;
        case '@':
            context.consume();
            context.pushResult(AMPERSAND);
            return;
        case '=':
            context.consume();

            if (context.next() == '=') {
                context.consume();
                context.pushResult(DOUBLE_EQUALS);
                return;
            } 

            context.pushResult(EQUALS);
            return;
        case '"':
            consume_string_literal(context);
            return;
        case '\'':
            consume_quoted_identifier(context);
            return;
        case '\n':
            context.consume();
            context.pushResult(NEWLINE, "\n");
            return;
        case '.':
            if (is_number(context.next(1))) {
                consume_number(context);
                return;
            }
            context.consume();
            context.pushResult(DOT);
            return;
        case '?':
            context.consume();
            context.pushResult(QUESTION);
            return;
        case '*':
            context.consume();
            if (context.next() == '=') {
                context.consume();
                context.pushResult(STAR_EQUALS);
                return;
            }

            context.pushResult(STAR);
            return;
        case '/':
            context.consume();
            if (context.next() == '=') {
                context.consume();
                context.pushResult(SLASH_EQUALS);
                return;
            }
            context.pushResult(SLASH);
            return;
        case '+':
            context.consume();
            context.pushResult(PLUS);
            return;
        case '-':
            if (is_number(context.next(1)) || context.next(1) == '.') {
                consume_number(context);
                return;
            }
            
            context.consume();
            if (context.next() == '>') {
                context.consume();
                context.pushResult(RIGHT_ARROW);
                return;
            }

            context.pushResult(MINUS);
            return;
    }

    // Fall through, consume the next letter as UNRECOGNIZED
    std::stringstream text;
    text << context.consume();
    context.pushResult(UNRECOGNIZED, text.str());
}

void consume_identifier(TokenizeContext &context)
{
    std::stringstream text;

    while (is_acceptable_inside_identifier(context.next())) {
        text << context.consume();
    }

    context.pushResult(IDENTIFIER, text.str());
}

void consume_whitespace(TokenizeContext &context)
{
    std::stringstream text;

    while (is_whitespace(context.next())) {
        text << context.consume();
    }

    context.pushResult(WHITESPACE, text.str());
}

void consume_number(TokenizeContext &context)
{
    std::stringstream text;

    bool minus_sign = false;
    bool dot_encountered = false;

    if (context.next() == '-') {
        text << context.consume();
        minus_sign = true;
    }

    while (true) {
        if (is_number(context.next())) {
            text << context.consume();
        }
        else if (context.next() == '.') {
            // If we've already encountered a dot, finish and don't consume
            // this one.
            if (dot_encountered)
                break;

            text << context.consume();
            dot_encountered = true;
        }
        else {
            break;
        }
    }

    if (dot_encountered) {
        context.pushResult(FLOAT, text.str());
    } else {
        context.pushResult(INTEGER, text.str());
    }
}

void consume_string_literal(TokenizeContext &context)
{
    std::stringstream text;

    // consume quote mark
    context.consume();

    while (context.next() != '"') {
        text << context.consume();
    }

    context.consume();

    context.pushResult(STRING, text.str());
}

void consume_quoted_identifier(TokenizeContext &context)
{
    std::stringstream text;

    // consume quote mark
    context.consume();

    while (is_acceptable_inside_identifier(context.next())) {
        text << context.consume();
    }

    context.pushResult(STRING, text.str());
}

} // namespace token
} // namespace circa
