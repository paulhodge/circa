// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "building.h"
#include "control_flow.h"
#include "kernel.h"
#include "function.h"
#include "hashtable.h"
#include "if_block.h"
#include "inspection.h"
#include "interpreter.h"
#include "parser.h"
#include "source_repro.h"
#include "stateful_code.h"
#include "string_type.h"
#include "names.h"
#include "term.h"
#include "type.h"

namespace circa {

static void list_inputs_to_pack_state(Block* block, int position, TermList* output);
static Term* append_final_pack_state(Block* block);

bool is_declared_state(Term* term)
{
    return term->function == FUNCS.declared_state;
}

bool does_callsite_have_implicit_state(Term* term)
{
    Term* func = term->function;

    if (!is_function(func))
        return false;
    Function* attrs = as_function(func);
    if (attrs == NULL)
        return false;

    Block* block = function_contents(func);
    return has_state_input(block);
}

static bool term_has_state_input(Term* term)
{
    for (int i=0; i < term->numInputs(); i++) {
        if (term_get_bool_input_prop(term, i, "state", false))
            return true;
    }
    return false;
}

void check_to_insert_implicit_state_input(Term* term)
{
    Block* block = term_get_function_details(term);

    if (term_has_state_input(term))
        return;

    Term* stateInput = find_state_input(block);

    if (stateInput == NULL || term_is_state_input(term, stateInput->index))
        return;

    int inputIndex = stateInput->index;

    Term* container = find_or_create_default_state_input(term->owningBlock);

    // Add a unpack_state() call
    Term* unpack = apply(term->owningBlock, FUNCS.unpack_state,
        TermList(container, term));
    hide_from_source(unpack);
    term->owningBlock->move(unpack, term->index);

    insert_input(term, inputIndex, unpack);
    set_bool(term->inputInfo(inputIndex)->properties.insert("state"), true);
    set_input_hidden(term, inputIndex, true);
}

void pack_any_open_state_vars(Block* block)
{
    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);
        if (term == NULL)
            continue;
        if (term->function == FUNCS.declared_state) {
            Term* result = block->get(term->name);

            // If this result already has a pack_state() term then leave it alone.
            if (find_user_with_function(result, FUNCS.pack_state) != NULL)
                continue;

            Term* pack = apply(block, FUNCS.pack_state,
                TermList(find_open_state_result(block), result, term));
            pack->setStringProp("field", as_cstring(unique_name(term)));
            block->move(pack, result->index + 1);
        }
    }
}

void block_update_state_type(Block* block)
{
    // Only run if state is dirty.
    if (!block_get_bool_property(block, sym_DirtyStateType, false))
        return;

    // Recreate the state type
    Type* type = create_compound_type();

    // TODO: give this new type a nice name

    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);
        if (term == NULL)
            continue;

        if (term->function != FUNCS.unpack_state || FUNCS.unpack_state == NULL)
            continue;

        Term* identifyingTerm = term->input(1);

        caValue* fieldName = unique_name(identifyingTerm);
        ca_assert(is_string(fieldName));
        ca_assert(!string_eq(fieldName, ""));

        compound_type_append_field(type, declared_type(term), as_cstring(fieldName));
    }

    block->stateType = type;
    block_remove_property(block, sym_DirtyStateType);

    // Might need to update any existing pack_state calls.
    block_update_pack_state_calls(block);
}

bool block_has_inline_state(Block* block)
{
    return block->stateType != NULL;
}

void block_mark_state_type_dirty(Block* block)
{
    set_bool(block_insert_property(block, sym_DirtyStateType), true);
}

static Term* append_final_pack_state(Block* block)
{
    TermList inputs;
    list_inputs_to_pack_state(block, block->length(), &inputs);
    Term* term = apply(block, FUNCS.pack_state, inputs);
    term->setBoolProp("final", true);
    return term;
}

// For the given field name
static Term* find_output_term_for_state_field(Block* block, const char* fieldName, int position)
{
    Term* result = find_from_unique_name(block, fieldName);

    // For declared state, the result is the last term with the given name
    if (result->function == FUNCS.declared_state) {
        return find_local_name(block, result->name.c_str(), position);
    }

    ca_assert(result != NULL);

    // This term might be the actual state result, or the state result might be
    // found in an extra output. Look around and see if this term has a stateful
    // extra output.
    for (int outputIndex=0;; outputIndex++) {
        Term* extraOutput = get_extra_output(result, outputIndex);
        if (extraOutput == NULL)
            break;

        if (is_state_output(extraOutput))
            return extraOutput;
    }

    return result;
}

static void list_inputs_to_pack_state(Block* block, int position, TermList* output)
{
    output->clear();

    if (block->stateType == NULL)
        return;

    for (int i=0; i < compound_type_get_field_count(block->stateType); i++) {

        const char* fieldName = compound_type_get_field_name(block->stateType, i);
        Term* result = find_output_term_for_state_field(block, fieldName, position);
        output->append(result);
    }
}

static bool should_have_preceeding_pack_state(Term* term)
{
    return has_escaping_control_flow(term);
}

void block_update_pack_state_calls(Block* block)
{
    if (block->stateType == NULL) {
        // No state type, make sure there's no pack_state call.
        // TODO: Handle this case properly (should search and destroy an existing pack_state call)
        return;
    }

    int stateOutputIndex = block->length() - 1 - find_state_output(block)->index;

    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);
        if (term == NULL)
            continue;

        if (term->function == FUNCS.pack_state) {
            // Update the inputs for this pack_state call
            TermList inputs;
            list_inputs_to_pack_state(block, i, &inputs);
            set_inputs(term, inputs);
        }

        else if (should_have_preceeding_pack_state(term)) {
            // Check if we need to insert a pack_state call
            Term* existing = term->input(stateOutputIndex);

            if (existing == NULL || existing->function != FUNCS.pack_state) {
                TermList inputs;
                list_inputs_to_pack_state(block, i, &inputs);
                if (inputs.length() != 0) {
                    Term* pack_state = apply(block, FUNCS.pack_state, inputs);
                    move_before(pack_state, term);

                    // Only set as an input for a non-minor block.
                    if (term->nestedContents == NULL || !is_minor_block(term->nestedContents)) {
                        set_input(term, stateOutputIndex, pack_state);
                        set_input_hidden(term, stateOutputIndex, true);
                        set_input_implicit(term, stateOutputIndex, true);
                    }

                    // Advance i to compensate for the term just added
                    i++;
                }
            }
        }
    }
}

void list_visible_declared_state(Block* block, TermList* output)
{
    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);
        if (is_declared_state(term))
            output->append(term);
    }

    if (is_minor_block(block) && get_parent_block(block) != NULL)
        list_visible_declared_state(get_parent_block(block), output);
}

Term* find_active_state_container(Block* block)
{
    // Check if there is already a stateful input

    // Special case for if-block: Look for a unpack_state_from_list call
    if (is_case_block(block)) {
        Term* existing = find_term_with_function(block, FUNCS.unpack_state_from_list);
        if (existing != NULL)
            return existing;
    }

    // Special case for for-block: Look for a unpack_state_from_list call

    Term* existing = find_state_input(block);
    if (existing != NULL)
        return existing;

    return NULL;
}

Term* find_or_create_default_state_input(Block* block)
{
    Term* existing = find_active_state_container(block);
    if (existing != NULL)
        return existing;

    // None yet, insert one
    Term* input = append_state_input(block);

    // Add a final pack_state call too
    append_final_pack_state(block);

    // And the state output
    append_state_output(block);

    return input;
}

Term* block_add_pack_state(Block* block)
{
    TermList inputs;
    list_inputs_to_pack_state(block, block->length(), &inputs);

    // Don't create anything if there are no state outputs
    if (inputs.length() == 0)
        return NULL;

    return apply(block, FUNCS.pack_state, inputs);
}

// Unpack a state value. Input 1 is the "identifying term" which is used as a key.
void unpack_state(caStack* stack)
{
    caValue* container = circa_input(stack, 0);
    Term* identifyingTerm = (Term*) circa_caller_input_term(stack, 1);

    if (identifyingTerm == NULL)
        return circa_output_error(stack, "input 1 is NULL");

    caValue* element = get_field(container, unique_name(identifyingTerm), NULL);

    if (element == NULL) {
        set_null(circa_output(stack, 0));
    } else {
        copy(element, circa_output(stack, 0));
    }
}

// Pack a state value. Each input will correspond with a slot in the block's state type.
void pack_state(caStack* stack)
{
    Term* caller = (Term*) circa_caller_term(stack);
    Block* block = caller->owningBlock;

    if (block->stateType == NULL)
        return;

    caValue* args = circa_input(stack, 0);
    caValue* output = circa_output(stack, 0);
    make(block->stateType, output);

    for (int i=0; i < circa_count(args); i++) {
        caValue* input = circa_index(args, i);
        caValue* dest = list_get(output, i);
        if (input == NULL)
            set_null(dest);
        else
            copy(input, dest);
    }
}

void declared_state_format_source(caValue* source, Term* term)
{
    if (!term->boolProp("syntax:stateKeyword", false)) {
        return format_term_source_default_formatting(source, term);
    }

    append_phrase(source, "state ", term, tok_State);

    if (term->hasProperty("syntax:explicitType")) {
        append_phrase(source, term->stringProp("syntax:explicitType",""),
                term, sym_TypeName);
        append_phrase(source, " ", term, tok_Whitespace);
    }

    append_phrase(source, term->name.c_str(), term, sym_TermName);

    Term* defaultValue = NULL;
    Block* initializer = NULL;

    if (term->input(2) != NULL)
        initializer = term->input(2)->nestedContents;

    if (initializer != NULL) {
        defaultValue = initializer->getFromEnd(0)->input(0);
        if (defaultValue->boolProp("hidden", false))
            defaultValue = defaultValue->input(0);
    }

    if (defaultValue != NULL) {
        append_phrase(source, " = ", term, sym_None);
        if (defaultValue->name != "")
            append_phrase(source, get_relative_name_at(term, defaultValue),
                    term, sym_TermName);
        else
            format_term_source(source, defaultValue);
    }
}

} // namespace circa
