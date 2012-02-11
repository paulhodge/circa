// Copyright (c) Paul Hodge. See LICENSE file for license terms.

namespace circa {
namespace null_t {

    std::string toString(TValue* value)
    {
        return "null";
    }
    void formatSource(StyledSource* source, Term* term)
    {
        append_phrase(source, "null", term, TK_NULL);
    }

    void setup_type(Type* type)
    {
        type->name = string_to_symbol("null");
        type->toString = toString;
        type->formatSource = formatSource;
    }
}
}
