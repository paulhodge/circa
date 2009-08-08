// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

namespace circa {
namespace div_function {

    void evaluate(Term* caller)
    {
        as_float(caller) = to_float(caller->input(0)) / to_float(caller->input(1));
    }

    void setup(Branch& kernel)
    {
        DIV_FUNC = import_function(kernel, evaluate, "div(float,float) : float");
    }
}
} // namespace circa
