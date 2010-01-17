// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <cmath>

#include <circa.h>

namespace circa {
namespace round_function {

    void evaluate_round(Term* caller)
    {
        float input = float_input(caller, 0);
        if (input > 0.0)
            set_int(caller, int(input + 0.5));
        else
            set_int(caller, int(input - 0.5));
    }

    void evaluate_floor(Term* caller)
    {
        set_int(caller, (int) std::floor(float_input(caller, 0)));
    }

    void evaluate_ceil(Term* caller)
    {
        set_int(caller, (int) std::ceil(float_input(caller, 0)));
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate_round, "round(number n) -> int; 'Return the integer that is closest to n' end");
        import_function(kernel, evaluate_floor, "floor(number n) -> int; 'Return the closest integer that is less than n' end");
        import_function(kernel, evaluate_ceil, "ceil(number n) -> int; 'Return the closest integer that is greater than n' end");
    }
}
}
