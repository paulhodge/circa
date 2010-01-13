// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace any_true_function {

    void evaluate(Term* caller)
    {
        Branch& input = as_branch(caller->input(0));

        bool result = false;
        for (int i=0; i < input.length(); i++)
            if (input[i]->asBool()) {
                result = true;
                break;
            }

        set_value_bool(caller, result);
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "any_true(List l) -> bool;"
                "'Return whether any of the items in l are true' end");
    }
}
} // namespace circa
