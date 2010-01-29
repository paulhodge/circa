// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace set_function {

    void evaluate(EvalContext*, Term* caller)
    {
        Branch &result = as_branch(caller);
        result.clear();

        for (int index=0; index < caller->numInputs(); index++) {
            set_t::add(result, caller->input(index));
        }
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "set(any...) -> Set");
    }
}
}
