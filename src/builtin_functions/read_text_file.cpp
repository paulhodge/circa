// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace read_text_file_function {

    void evaluate(EvalContext*, Term* caller)
    {
        std::string filename = as_string(caller->input(0));
        set_str(caller, read_text_file(filename));
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "read_text_file(string) -> string");
    }
}
} // namespace circa
