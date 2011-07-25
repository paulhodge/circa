// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "builtins.h"
#include "building.h"
#include "evaluation.h"
#include "introspection.h"
#include "term.h"
#include "type.h"
#include "type_inference.h"

#include "types/list.h"

namespace circa {

Term* find_common_type(TermList const& list)
{
    if (list.length() == 0)
        return ANY_TYPE;

    // Check if every type in this list is the same.
    bool all_equal = true;
    for (int i=1; i < list.length(); i++) {
        if (list[0] != list[i]) {
            all_equal = false;
            break;
        }
    }

    if (all_equal)
        return list[0];

    // Special case, allow ints to go into floats
    bool all_are_ints_or_floats = true;
    for (int i=0; i < list.length(); i++) {
        if ((list[i] != INT_TYPE) && (list[i] != FLOAT_TYPE)) {
            all_are_ints_or_floats = false;
            break;
        }
    }

    if (all_are_ints_or_floats)
        return FLOAT_TYPE;

    // Another special case, if all types are lists then use LIST_TYPE
    bool all_are_lists = true;
    for (int i=0; i < list.length(); i++) {
        if (!is_list_based_type(unbox_type(list[i])))
            all_are_lists = false;
    }

    if (all_are_lists)
        return LIST_TYPE;

    // Otherwise give up
    return ANY_TYPE;
}

Term* find_type_of_get_index(Term* listTerm)
{
    if (listTerm->function == RANGE_FUNC)
        return INT_TYPE;

    if (listTerm->function == LIST_FUNC) {
        TermList inputTypes;
        for (int i=0; i < listTerm->numInputs(); i++)
            inputTypes.append(listTerm->input(i)->type);
        return find_common_type(inputTypes);
    }

    if (listTerm->function == COPY_FUNC)
        return find_type_of_get_index(listTerm->input(0));

    if (is_list_based_type(unbox_type(listTerm->type))) {
        Branch& prototype = type_t::get_prototype(unbox_type(listTerm->type));
        TermList types;
        for (int i=0; i < prototype.length(); i++)
            types.append(prototype[i]->type);
        return find_common_type(types);
    }

    // Unrecognized
    return ANY_TYPE;
}

Term* statically_infer_type_of_get_index(Branch& branch, Term* term)
{
    if (term->function == RANGE_FUNC)
        create_type_value(branch, &INT_T);

#if 0
    if (listTerm->function == LIST_FUNC) {
        TermList inputTypes;
        for (int i=0; i < listTerm->numInputs(); i++)
            inputTypes.append(listTerm->input(i)->type);
        return find_common_type(inputTypes);
    }

    if (listTerm->function == COPY_FUNC)
        return find_type_of_get_index(listTerm->input(0));

    if (is_list_based_type(unbox_type(listTerm->type))) {
        Branch& prototype = type_t::get_prototype(unbox_type(listTerm->type));
        TermList types;
        for (int i=0; i < prototype.length(); i++)
            types.append(prototype[i]->type);
        return find_common_type(types);
    }
#endif

    // Unrecognized
    return ANY_TYPE;
}

Term* statically_infer_type(Branch& branch, Term* term)
{
    if (term->function == GET_INDEX_FUNC)
        return statically_infer_type_of_get_index(branch, term);
    
    // Give up
    std::cout << "statically_infer_type didn't understand: "
        << term->function->name << std::endl;
    return create_symbol_value(branch, &UNKNOWN_SYMBOL);
}

Term* statically_infer_length_func(Branch& branch, Term* term)
{
    Term* input = term->input(0);
    if (input->function == COPY_FUNC)
        return statically_infer_length_func(branch, input->input(0));

    if (input->function == LIST_FUNC)
        return create_int(branch, input->numInputs());

    if (input->function == LIST_APPEND_FUNC) {
        Term* leftLength = apply(branch, LENGTH_FUNC, TermList(input->input(0)));
        return apply(branch, ADD_FUNC, TermList(
                    statically_infer_length_func(branch, leftLength),
                    create_int(branch, 1)));
    }

    // Give up
    std::cout << "statically_infer_length_func didn't understand: "
        << input->function->name << std::endl;
    return create_symbol_value(branch, &UNKNOWN_SYMBOL);
}

Term* statically_infer_result(Branch& branch, Term* term)
{
    if (term->function == LENGTH_FUNC)
        return statically_infer_length_func(branch, term);

    if (term->function == TYPE_FUNC)
        return statically_infer_type(branch, term->input(0));

    // Function not recognized
    std::cout << "statically_infer_result didn't recognize: "
        << term->function->name << std::endl;
    return create_symbol_value(branch, &UNKNOWN_SYMBOL);
}

void statically_infer_result(Term* term, TaggedValue* result)
{
    Branch scratch;

    Term* resultTerm = statically_infer_result(scratch, term);

    if (is_value(resultTerm))
        copy(resultTerm, result);
    else {
        EvalContext context;
        evaluate_minimum(&context, resultTerm, result);
    }
}

} // namespace circa
