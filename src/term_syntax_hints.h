// Copyright 2008 Paul Hodge

#ifndef CIRCA_TERM_SYNTAX_HINTS_INCLUDED
#define CIRCA_TERM_SYNTAX_HINTS_INCLUDED

#include "common_headers.h"

namespace circa {

struct TermSyntaxHints
{
    struct InputSyntax {
        enum Style {
            UNKNOWN_STYLE=0,
            BY_NAME,
            BY_SOURCE };

        Style style;
        std::string name;
        std::string precedingWhitespace;
        std::string followingWhitespace;

        InputSyntax() : style(UNKNOWN_STYLE) {}

        void unknownStyle()
        {
            style = UNKNOWN_STYLE;
            name = "";
        }

        void bySource()
        {
            style = BY_SOURCE;
            name = "";
        }

        void byName(std::string const& name)
        {
            style = BY_NAME;
            this->name = name;
        }
    };

    enum DeclarationStyle {
        UNKNOWN_DECLARATION_STYLE=0,
        FUNCTION_CALL,
        INFIX,
        DOT_CONCATENATION,
        ARROW_CONCATENATION,
        LITERAL_VALUE,
        SPECIAL_CASE // includes if-statement
    };

    typedef std::vector<InputSyntax> InputSyntaxList;

    // Members:
    int line;
    int startChar;
    int endChar;
    DeclarationStyle declarationStyle;
    InputSyntaxList inputSyntax;
    int parens;
    bool occursInsideAnExpression;
    std::string functionName;
    std::string precedingWhitespace;
    std::string followingWhitespace;

    TermSyntaxHints() :
        line(0),
        startChar(0),
        endChar(0),
        declarationStyle(UNKNOWN_DECLARATION_STYLE),
        parens(0),
        occursInsideAnExpression(false)
    {}

    InputSyntax& getInputSyntax(unsigned int index) {
        while (index >= inputSyntax.size())
            inputSyntax.push_back(InputSyntax());

        return inputSyntax[index];
    }

    void setInputSyntax(unsigned int index, InputSyntax const& syntax)
    {
        assert(index < 1000);

        while (index >= inputSyntax.size())
            inputSyntax.push_back(InputSyntax());

        inputSyntax[index] = syntax;
    }
};

}

#endif
