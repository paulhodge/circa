// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

// importing.cpp : Functions to help import C functions into Circa.

#pragma once

#include "common_headers.h"

#include "function.h"
#include "token_stream.h"

namespace circa {

CA_FUNCTION(empty_evaluate);
CA_FUNCTION(empty_evaluate_no_touch_output);

// Create a Circa function, using the given C evaluation function, and
// a header in Circa-syntax.
//
// Example function header: "function do-something(int, string) -> int"
Term* import_function(Branch& branch, EvaluateFunc func, std::string const& header);

Term* import_member_function(Type* type, EvaluateFunc evaluate,
        std::string const& headerText);

Term* import_member_function(Term* type, EvaluateFunc evaluate,
        std::string const& headerText);

// Install an evaluate function into an existing function object.
void install_function(Term* function, EvaluateFunc evaluate);

Term* import_type(Branch& branch, Type* type);

} // namespace circa
