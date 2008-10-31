// Copyright 2008 Andrew Fischer

namespace print_function {

    void evaluate(Term* caller)
    {
        std::cout << as_string(caller->inputs[0]) << std::endl;
    }

    void setup(Branch& kernel) {
        import_c_function(kernel, evaluate,
             "function print(string)");
    }
}
