// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

struct ForLoopContext
{
    bool discard;
    bool breakCalled;
    bool continueCalled;

    ForLoopContext() : discard(false), breakCalled(false), continueCalled(false) {}
};

Term* get_for_loop_iterator(Term* forTerm);
bool for_loop_modifies_list(Term* forTerm);
Term* setup_for_loop_iterator(Term* forTerm, const char* name);
void finish_for_loop(Term* forTerm);

Term* find_enclosing_for_loop(Term* term);

void for_loop_update_output_index(Term* forTerm);

CA_FUNCTION(evaluate_for_loop);

} // namespace circa
