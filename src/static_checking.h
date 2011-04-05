// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"
#include "types/list.h"

namespace circa {

void check_for_static_errors(List* result, Branch& branch);

// Print each static error to 'out'. Returns true if there were any static errors.
bool print_static_errors_formatted(List* result, std::ostream& out);

bool has_static_error(Term* term);
bool has_static_errors(Branch& branch);
int count_static_errors(Branch& branch);

bool print_static_errors_formatted(Branch& branch, std::ostream& out);
void print_static_error(Term* term, std::ostream& out);

std::string get_static_errors_formatted(Branch& branch);
std::string get_static_error_message(Term* term);

} // namespace circ
