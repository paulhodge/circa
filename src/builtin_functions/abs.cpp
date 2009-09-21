// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved

#include <circa.h>

namespace circa {
namespace abs_function {

    void evaluate(Term* caller)
    {
        as_float(caller) = std::abs(to_float(caller->input(0)));
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "abs(number n) : number;"
                "'Return the absolute value of n' end");
    }
}
}
