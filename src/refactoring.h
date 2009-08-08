// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#ifndef CIRCA_REFACTORING_INCLUDED
#define CIRCA_REFACTORING_INCLUDED

#include "common_headers.h"

namespace circa {

// Change this term to have the given function
void change_function(Term* term, Term* newFunction);

void unsafe_change_type(Term* term, Term* type);
void change_type(Term* term, Term* type);
void specialize_type(Term* term, Term* type);

// Rename term, modify the name binding of the owning branch if necessary
void rename(Term* term, std::string const& name);

// Remove this term from its owning branch, and add it to this branch instead.
// This will create a NULL in the owning branch.
void steal_term(Term* term, Branch& newHome);

} // namespace circa

#endif
