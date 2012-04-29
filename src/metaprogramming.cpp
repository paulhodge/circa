// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"
#include "circa/circa.h"

#include "building.h"
#include "code_iterators.h"
#include "evaluation.h"
#include "importing.h"
#include "introspection.h"
#include "kernel.h"
#include "metaprogramming.h"
#include "source_repro.h"
#include "stateful_code.h"
#include "static_checking.h"
#include "term.h"
#include "tagged_value.h"
#include "type.h"

#include "value_iterator.h"

namespace circa {
    
void set_term_ref(caValue* val, Term* term)
{
    change_type(val, &REF_T);
    val->value_data.ptr = term;
}

Term* as_term_ref(caValue* val)
{
    return (Term*) val->value_data.ptr;
}

void branch_ref(caStack* stack)
{
    Term* input0 = (Term*) circa_caller_input_term(stack, 0);
    Branch* branch = input0->nestedContents;
    gc_mark_object_referenced(&branch->header);
    set_branch(circa_output(stack, 0), branch);
}

void term_ref(caStack* stack)
{
    caTerm* term = circa_caller_input_term(stack, 0);
    set_term_ref(circa_output(stack, 0), (Term*) term);
}

// Returns the corresponding term inside newBranch, if found.
// Returns 'term' if the translation does not apply (term is not found inside
// oldBranch).
// Returns NULL if the translation does apply, but a corresponding term cannot be found.
Term* translate_term_across_branches(Term* term, Branch* oldBranch, Branch* newBranch)
{
    if (!term_is_child_of_branch(term, oldBranch))
        return term;

    Value relativeName;
    get_relative_name(term, oldBranch, &relativeName);
    return find_from_relative_name(&relativeName, newBranch);
}

void update_all_code_references(caValue* value, Branch* oldBranch, Branch* newBranch)
{
    for (ValueIterator it(value); it.unfinished(); it.advance()) {
        caValue* val = *it;
        if (is_ref(val)) {
            set_term_ref(val, translate_term_across_branches(as_term_ref(val),
                oldBranch, newBranch));
            
        } else if (is_branch(val)) {

            Term* oldTerm = as_branch(val)->owningTerm;
            if (oldTerm == NULL)
                continue;

            Term* newTerm = translate_term_across_branches(oldTerm, oldBranch, newBranch);
            if (newTerm == NULL) {
                set_branch(val, NULL);
                continue;
            }

            set_branch(val, newTerm->nestedContents);
        }
    }
}

void get_relative_name(Term* term, Branch* relativeTo, caValue* nameOutput)
{
    set_list(nameOutput, 0);

    // Walk upwards and build the name, stop when we reach relativeTo.
    // The output list will be reversed but we'll fix that.

    while (true) {
        set_string(list_append(nameOutput), get_unique_name(term));

        if (term->owningBranch == relativeTo) {
            break;
        }

        term = get_parent_term(term);

        // If term is null, then it wasn't really a child of relativeTo
        if (term == NULL) {
            set_null(nameOutput);
            return;
        }
    }

    // Fix output list
    list_reverse(nameOutput);
}

Term* find_from_relative_name(caValue* name, Branch* relativeTo)
{
    if (is_null(name))
        return NULL;

    Term* term = NULL;
    for (int index=0; index < list_length(name); index++) {
        term = find_from_unique_name(relativeTo, as_cstring(list_get(name, index)));

        if (term == NULL)
            return NULL;

        relativeTo = term->nestedContents;
    }
    return term;
}

void Branch__dump(caStack* stack)
{
    dump(as_branch(circa_input(stack, 0)));
}

void Branch__input(caStack* stack)
{
    Branch* branch = as_branch(circa_input(stack, 0));
    set_term_ref(circa_output(stack, 0),
        get_input_placeholder(branch, circa_int_input(stack, 1)));
}
void Branch__inputs(caStack* stack)
{
    Branch* branch = as_branch(circa_input(stack, 0));
    caValue* output = circa_output(stack, 0);
    set_list(output, 0);
    for (int i=0;; i++) {
        Term* term = get_input_placeholder(branch, i);
        if (term == NULL)
            break;
        set_term_ref(list_append(output), term);
    }
}
void Branch__output(caStack* stack)
{
    Branch* branch = as_branch(circa_input(stack, 0));
    set_term_ref(circa_output(stack, 0),
        get_output_placeholder(branch, circa_int_input(stack, 1)));
}
void Branch__outputs(caStack* stack)
{
    Branch* branch = as_branch(circa_input(stack, 0));
    caValue* output = circa_output(stack, 0);
    set_list(output, 0);
    for (int i=0;; i++) {
        Term* term = get_output_placeholder(branch, i);
        if (term == NULL)
            break;
        set_term_ref(list_append(output), term);
    }
}

void Branch__format_source(caStack* stack)
{
    Branch* branch = as_branch(circa_input(stack, 0));

    caValue* output = circa_output(stack, 0);
    circa_set_list(output, 0);
    format_branch_source((StyledSource*) output, branch);
}

void Branch__has_static_error(caStack* stack)
{
    Branch* branch = as_branch(circa_input(stack, 0));
    set_bool(circa_output(stack, 0), has_static_errors_cached(branch));
}

void Branch__get_static_errors(caStack* stack)
{
    Branch* branch = as_branch(circa_input(stack, 0));

    if (is_null(&branch->staticErrors))
        set_list(circa_output(stack, 0), 0);
    else
        copy(&branch->staticErrors, circa_output(stack, 0));
}

void Branch__get_static_errors_formatted(caStack* stack)
{
    Branch* branch = as_branch(circa_input(stack, 0));
    if (branch == NULL)
        return circa_output_error(stack, "NULL branch");

    if (is_null(&branch->staticErrors))
        set_list(circa_output(stack, 0), 0);

    caValue* errors = &branch->staticErrors;
    caValue* out = circa_output(stack, 0);
    set_list(out, circa_count(errors));
    for (int i=0; i < circa_count(out); i++)
        format_static_error(circa_index(errors, i), circa_index(out, i));
}

void Branch__call(caStack* stack)
{
    Branch* branch = as_branch(circa_input(stack, 0));
    if (branch == NULL)
        return circa_output_error(stack, "NULL branch");

    caValue* inputs = circa_input(stack, 1);
    push_frame_with_inputs(stack, branch, inputs);
}

// Reflection

void Branch__terms(caStack* stack)
{
    Branch* branch = as_branch(circa_input(stack, 0));

    caValue* out = circa_output(stack, 0);
    set_list(out, branch->length());

    for (int i=0; i < branch->length(); i++)
        set_term_ref(circa_index(out, i), branch->get(i));
}

void Branch__get_term(caStack* stack)
{
    Branch* branch = as_branch(circa_input(stack, 0));

    int index = circa_int_input(stack, 1);
    set_term_ref(circa_output(stack, 0), branch->get(index));
}

bool is_considered_config(Term* term)
{
    if (term == NULL) return false;
    if (term->name == "") return false;
    if (!is_value(term)) return false;
    if (is_declared_state(term)) return false;
    if (is_hidden(term)) return false;
    if (is_function(term)) return false;

    // ignore branch-based types
    //if (is_branch(term)) return false;
    if (is_type(term)) return false;

    return true;
}

void Branch__list_configs(caStack* stack)
{
    Branch* branch = as_branch(circa_input(stack, 0));

    caValue* output = circa_output(stack, 0);

    for (int i=0; i < branch->length(); i++) {
        Term* term = branch->get(i);
        if (is_considered_config(term))
            set_term_ref(circa_append(output), term);
    }
}

void Branch__functions(caStack* stack)
{
    Branch* branch = as_branch(circa_input(stack, 0));

    caValue* output = circa_output(stack, 0);
    set_list(output, 0);

    for (BranchIteratorFlat it(branch); it.unfinished(); it.advance()) {
        Term* term = *it;
        if (is_function(term)) {
            set_function(list_append(output), as_function(term));
        }
    }
}

void Branch__file_signature(caStack* stack)
{
    Branch* branch = as_branch(circa_input(stack, 0));
    List* fileOrigin = branch_get_file_origin(branch);
    if (fileOrigin == NULL)
        set_null(circa_output(stack, 0));
    else
    {
        List* output = set_list(circa_output(stack, 0), 2);
        copy(fileOrigin->get(1), output->get(0));
        copy(fileOrigin->get(2), output->get(1));
    }
}

void Branch__statements(caStack* stack)
{
    Branch* branch = (Branch*) circa_branch(circa_input(stack, 0));

    caValue* out = circa_output(stack, 0);

    circa_set_list(out, 0);

    for (int i=0; i < branch->length(); i++)
        if (is_statement(branch->get(i)))
            circa_set_term(circa_append(out), (caTerm*) branch->get(i));
}

void Branch__link(caStack* stack)
{
    Branch* self = (Branch*) circa_branch(circa_input(stack, 0));
    Branch* source = (Branch*) circa_branch(circa_input(stack, 1));

    branch_link_missing_functions(self, source);
}

void Term__name(caStack* stack)
{
    Term* t = circa_input(stack, 0)->asRef();
    if (t == NULL)
        return circa_output_error(stack, "NULL reference");
    set_string(circa_output(stack, 0), t->name);
}
void Term__to_string(caStack* stack)
{
    Term* t = circa_input(stack, 0)->asRef();
    if (t == NULL)
        return circa_output_error(stack, "NULL reference");
    set_string(circa_output(stack, 0), circa::to_string(term_value(t)));
}
void Term__to_source_string(caStack* stack)
{
    Term* t = circa_input(stack, 0)->asRef();
    if (t == NULL)
        return circa_output_error(stack, "NULL reference");
    set_string(circa_output(stack, 0), get_term_source_text(t));
}
void Term__function(caStack* stack)
{
    Term* t = circa_input(stack, 0)->asRef();
    if (t == NULL)
        return circa_output_error(stack, "NULL reference");
    set_function(circa_output(stack, 0), as_function(t->function));
}
void Term__type(caStack* stack)
{
    Term* t = circa_input(stack, 0)->asRef();
    if (t == NULL)
        return circa_output_error(stack, "NULL reference");
    set_type(circa_output(stack, 0), t->type);
}
void Term__assign(caStack* stack)
{
    Term* target = circa_input(stack, 0)->asRef();
    if (target == NULL) {
        circa_output_error(stack, "NULL reference");
        return;
    }

    caValue* source = circa_input(stack, 1);

    if (!cast_possible(source, declared_type(target))) {
        circa_output_error(stack, "Can't assign, type mismatch");
        return;
    }

    circa::copy(source, term_value(target));
}
void Term__value(caStack* stack)
{
    Term* target = circa_input(stack, 0)->asRef();
    if (target == NULL) {
        circa_output_error(stack, "NULL reference");
        return;
    }

    copy(term_value(target), circa_output(stack, 0));
}

int tweak_round(double a) {
    return int(a + 0.5);
}

void Term__tweak(caStack* stack)
{
    Term* t = circa_input(stack, 0)->asRef();
    if (t == NULL)
        return circa_output_error(stack, "NULL reference");

    int steps = tweak_round(circa_input(stack, 1)->toFloat());

    caValue* val = term_value(t);

    if (steps == 0)
        return;

    if (is_float(val)) {
        float step = get_step(t);

        // Do the math like this so that rounding errors are not accumulated
        float new_value = (round(as_float(val) / step) + steps) * step;
        set_float(val, new_value);

    } else if (is_int(val))
        set_int(val, as_int(val) + steps);
    else
        circa_output_error(stack, "Ref is not an int or number");
}

void Term__asint(caStack* stack)
{
    Term* t = circa_input(stack, 0)->asRef();
    if (t == NULL) {
        circa_output_error(stack, "NULL reference");
        return;
    }
    if (!is_int(term_value(t))) {
        circa_output_error(stack, "Not an int");
        return;
    }
    set_int(circa_output(stack, 0), as_int(term_value(t)));
}
void Term__asfloat(caStack* stack)
{
    Term* t = circa_input(stack, 0)->asRef();
    if (t == NULL) {
        circa_output_error(stack, "NULL reference");
        return;
    }
    
    set_float(circa_output(stack, 0), to_float(term_value(t)));
}
void Term__input(caStack* stack)
{
    Term* t = circa_input(stack, 0)->asRef();
    if (t == NULL) {
        circa_output_error(stack, "NULL reference");
        return;
    }
    int index = circa_input(stack, 1)->asInt();
    if (index >= t->numInputs())
        set_term_ref(circa_output(stack, 0), NULL);
    else
        set_term_ref(circa_output(stack, 0), t->input(index));
}
void Term__inputs(caStack* stack)
{
    Term* t = circa_input(stack, 0)->asRef();
    if (t == NULL)
        return circa_output_error(stack, "NULL reference");

    caValue* output = circa_output(stack, 0);
    circa_set_list(output, t->numInputs());

    for (int i=0; i < t->numInputs(); i++)
        set_term_ref(circa_index(output, i), t->input(i));
}
void Term__num_inputs(caStack* stack)
{
    Term* t = circa_input(stack, 0)->asRef();
    if (t == NULL) {
        circa_output_error(stack, "NULL reference");
        return;
    }
    set_int(circa_output(stack, 0), t->numInputs());
}

void Term__source_location(caStack* stack)
{
    Term* t = circa_input(stack, 0)->asRef();
    if (t == NULL)
        return circa_output_error(stack, "NULL reference");

    circa_set_vec4(circa_output(stack, 0),
        t->sourceLoc.col, t->sourceLoc.line,
        t->sourceLoc.colEnd, t->sourceLoc.lineEnd);
}
void Term__global_id(caStack* stack)
{
    Term* t = circa_input(stack, 0)->asRef();
    if (t == NULL)
        return circa_output_error(stack, "NULL reference");

    set_int(circa_output(stack, 0), t->id);
}
void Term__properties(caStack* stack)
{
    Term* t = circa_input(stack, 0)->asRef();
    if (t == NULL)
        return circa_output_error(stack, "NULL reference");
    circa::copy(&t->properties, circa_output(stack, 0));
}
void Term__property(caStack* stack)
{
    Term* t = circa_input(stack, 0)->asRef();
    if (t == NULL)
        return circa_output_error(stack, "NULL reference");

    const char* key = circa_string_input(stack, 1);

    caValue* value = term_get_property(t, key);

    if (value == NULL)
        set_null(circa_output(stack, 0));
    else
        circa::copy(value, circa_output(stack, 0));
}

void metaprogramming_install_functions(Branch* kernel)
{
    static const ImportRecord records[] = {
        {"term_ref", term_ref},
        {"branch_ref", branch_ref},
        {"Branch.input", Branch__input},
        {"Branch.inputs", Branch__inputs},
        {"Branch.output", Branch__output},
        {"Branch.outputs", Branch__outputs},
        {"Branch.dump", Branch__dump},
        {"Branch.call", Branch__call},
        {"Branch.file_signature", Branch__file_signature},
        {"Branch.statements", Branch__statements},
        {"Branch.format_source", Branch__format_source},
        {"Branch.get_term", Branch__get_term},
        {"Branch.get_static_errors", Branch__get_static_errors},
        {"Branch.get_static_errors_formatted", Branch__get_static_errors_formatted},
        {"Branch.has_static_error", Branch__has_static_error},
        {"Branch.list_configs", Branch__list_configs},
        {"Branch.functions", Branch__functions},
        {"Branch.terms", Branch__terms},
        {"Branch.link", Branch__link},
        {"Term.name", Term__name},
        {"Term.to_string", Term__to_string},
        {"Term.to_source_string", Term__to_source_string},
        {"Term.function", Term__function},
        {"Term.get_type", Term__type},
        {"Term.assign", Term__assign},
        {"Term.value", Term__value},
        {"Term.tweak", Term__tweak},
        {"Term.asint", Term__asint},
        {"Term.asfloat", Term__asfloat},
        {"Term.input", Term__input},
        {"Term.inputs", Term__inputs},
        {"Term.num_inputs", Term__num_inputs},
        {"Term.source_location", Term__source_location},
        {"Term.global_id", Term__global_id},
        {"Term.properties", Term__properties},
        {"Term.property", Term__property},
    
        {NULL, NULL}
    };

    install_function_list(kernel, records);
}

} // namespace circa
