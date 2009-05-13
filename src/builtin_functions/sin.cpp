// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace sin_function {

    void evaluate(Term* caller)
    {
        float input = to_float(caller->input(0));
        as_float(caller) = sin(input);
    }

    /*
    void generateFeedback(Branch& branch, Term* subject, Term* desired)
    {
        Term* inputDesired = float_value(&branch, asin(to_float(desired)));
        generate_feedback(branch, subject->input(0), inputDesired);
    }
    */

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate, "sin(float) : float");
        as_function(main_func).pureFunction = true;
    }
}
}
