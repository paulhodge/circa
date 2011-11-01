// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

#include "types/ref.h"

namespace circa {
namespace vectorize_vv_function {

    Type* specializeType(Term* caller)
    {
        Type* lhsType = caller->input(0)->type;
        if (is_list_based_type(lhsType))
            return lhsType;
        return &LIST_T;
    }

    CA_FUNCTION(evaluate)
    {
        Branch* contents = nested_contents(CALLER);
        TaggedValue input0, input1;

        if (num_elements(INPUT(0)) != num_elements(INPUT(1)))
            return error_occurred(CONTEXT, CALLER, "Input lists have different lengths");

        copy(INPUT(0), &input0);
        copy(INPUT(1), &input1);
        int listLength = input0.numElements();

        Term* input0_placeholder = contents->get(0);
        Term* input1_placeholder = contents->get(1); 
        Term* content_output = contents->get(2); 

        push_frame(CONTEXT, contents);

        // Prepare output
        TaggedValue outputTv;
        List* output = set_list(&outputTv, listLength);

        // Evaluate vectorized call, once for each input
        for (int i=0; i < listLength; i++) {
            // Copy inputs into placeholder
            swap(input0.getIndex(i),
                top_frame(CONTEXT)->registers[input0_placeholder->index]);
            swap(input1.getIndex(i),
                top_frame(CONTEXT)->registers[input1_placeholder->index]);

            evaluate_single_term(CONTEXT, content_output);

            // Save output
            swap(top_frame(CONTEXT)->registers[content_output->index],
                output->get(i));
        }

        pop_frame(CONTEXT);

        swap(output, OUTPUT);
    }

    void post_input_change(Term* term)
    {
        // Update generated code
        Branch* contents = nested_contents(term);
        contents->clear();

        TaggedValue* funcParam = &as_function(term->function)->parameter;
        if (funcParam == NULL || !is_ref(funcParam))
            return;

        Term* func = as_ref(funcParam);
        Term* left = term->input(0);
        Term* right = term->input(1);

        if (func == NULL || left == NULL || right == NULL)
            return;

        Term* leftPlaceholder = apply(contents, INPUT_PLACEHOLDER_FUNC, TermList());
        change_declared_type(leftPlaceholder, infer_type_of_get_index(left));

        Term* rightPlaceholder = apply(contents, INPUT_PLACEHOLDER_FUNC, TermList());
        change_declared_type(rightPlaceholder, infer_type_of_get_index(right));

        apply(contents, func, TermList(leftPlaceholder, rightPlaceholder));
    }

    void setup(Branch* kernel)
    {
        Term* func = import_function(kernel, evaluate,
                "vectorize_vv(List,List) -> List");
        as_function(func)->specializeType = specializeType;
        as_function(func)->postInputChange = post_input_change;
        as_function(func)->createsStackFrame = true;
    }
}
} // namespace circa
