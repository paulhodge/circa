// Copyright 2008 Andrew Fischer

#include <circa.h>

namespace circa {
namespace unique_id_function {

    void evaluate(Term* caller)
    {
        static int nextId = 1;
        as_int(caller) = nextId++;
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "unique_id() : int");
    }
}
}
