// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "branch.h"
#include "circa.h"
#include "introspection.h"

namespace circa {
namespace branch_to_source_text_function {

    void evaluate(Term* caller)
    {
        Branch& branch = as_branch(caller->input(0));
        std::stringstream result;

        for (int i=0; i < branch.length(); i++) {

            Term* term = branch[i];

            if (!should_print_term_source_line(term))
                continue;

            result << get_term_source(term);

            if (i+1 < branch.length()) {
                result << "\n";
            }
        }

        as_string(caller) = result.str();
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate,
                "branch_to_source_text(Branch) : string");
    }
}
}
