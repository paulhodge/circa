// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

// Examine 'function' and 'inputs' and returns a result term.
Term* apply(Branch& branch, Term* function, RefList const& inputs,
    std::string const& name="");

// Find the named function in this branch, and then call the above apply.
Term* apply(Branch& branch, std::string const& functionName, 
                 RefList const& inputs, std::string const& name="");

// Create a duplicate of the given term.
// If 'copyBranches' is false, don't copy branch state. It's assumed that the
// caller will do this. This functionality is used by duplicate_branch
Term* create_duplicate(Branch& branch, Term* original, std::string const& name="",
        bool copyBranches=true);

void set_input(Term* term, int index, Term* input);
void set_inputs(Term* term, RefList const& inputs);
int get_input_relative_scope(Term* term, int index);
void post_input_change(Term* term);

bool is_actually_using(Term* user, Term* usee);

// This finds all the terms which have this term as a user, and removes it from
// their user list. This is appropriate when you want to delete 'term'.
void remove_from_users(Term* term);

// Create a new value term with the given type.
Term* create_value(Branch& branch, Term* type, std::string const& name="");
Term* create_value(Branch& branch, std::string const& typeName, std::string const& name="");

Term* create_stateful_value(Branch& branch, Term* type, Term* defaultValue,
        std::string const& name);

// Create values with a specified value.
Term* create_string(Branch& branch, std::string const& s, std::string const& name="");
Term* create_int(Branch& branch, int i, std::string const& name="");
Term* create_float(Branch& branch, float f, std::string const& name="");
Term* create_bool(Branch& branch, bool b, std::string const& name="");
Term* create_ref(Branch& branch, Term* ref, std::string const& name="");
Term* create_void(Branch& branch, std::string const& name="");
Term* create_list(Branch& branch, std::string const& name="");
Branch& create_branch(Branch& owner, std::string const& name="");
Branch& create_namespace(Branch&, std::string const& name);
Term* create_type(Branch& branch, std::string name="");
Term* create_empty_type(Branch& branch, std::string name);
Term* duplicate_value(Branch& branch, Term* term);

// In this context, "procure" means "return the existing thing if it already exists, and
// create it if it doesn't exist." Procure functions are idempotent.
Term* procure_value(Branch& branch, Term* type, std::string const& name);

Term* procure_int(Branch& branch, std::string const& name);
Term* procure_float(Branch& branch, std::string const& name);
Term* procure_bool(Branch& branch, std::string const& name);

void set_step(Term* term, float step);
float get_step(Term* term);

// Call the term's postCompile handler, if there is one.
void post_compile_term(Term* term);

// Add a finish_minor_branch() term to this branch, if needed.
void finish_minor_branch(Branch& branch);

void check_to_add_branch_finish_term(Branch& branch, int previousLastTerm);
void update_branch_finish_term(Term* term);

} // namespace circa
