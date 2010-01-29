// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace log_function {

    void evaluate(EvalContext*, Term* caller)
    {
        set_float(caller, std::log(float_input(caller,0)));
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "log(number) -> number;"
            "'Natural log function' end");
    }
}
}
