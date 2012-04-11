// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace eval_context_t {

    EvalContext* get(caValue* value)
    {
        return ((EvalContext*) value->value_data.ptr);
    }

    void visitHeap(Type*, caValue* value, Type::VisitHeapCallback callback, caValue* visitContext)
    {
        EvalContext* context = get(value);
        Value relIdent;

        set_string(&relIdent, "state");
        callback(&context->state, &relIdent, visitContext);

        set_string(&relIdent, "messages");
        callback(&context->messages, &relIdent, visitContext);
    }

    void setup_type(Type* type)
    {
        type->name = name_from_string("EvalContext");
        type->visitHeap = visitHeap;
    }
}
} // namespace circa

