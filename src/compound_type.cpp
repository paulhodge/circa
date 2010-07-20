// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "builtin_types.h"
#include "builtin_types/list.h"
#include "building.h"
#include "term.h"
#include "type.h"

namespace circa {

void initialize_compound_type(Type* type)
{
    list_t::setup_type(type);
}

void initialize_compound_type(Term* term)
{
    initialize_compound_type(type_contents(term));
}

Term* create_compound_type(Branch& branch, std::string const& name)
{
    Term* term = create_type(branch, name);
    initialize_compound_type(term);
    type_contents(term)->name = name;
    return term;
}

}