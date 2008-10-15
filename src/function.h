#ifndef CIRCA_FUNCTION_INCLUDED
#define CIRCA_FUNCTION_INCLUDED

#include "common_headers.h"

#include "term.h"
#include "branch.h"

namespace circa {

#define INPUT_PLACEHOLDER_PREFIX "#input-"
#define OUTPUT_PLACEHOLDER_NAME "#output"

struct Function
{
    typedef void (*InitializeFunc)(Term* caller);
    typedef void (*EvaluateFunc)(Term* caller);

    ReferenceList inputTypes;
    Term* outputType;

    Term* stateType;

    bool pureFunction;
    bool variableArgs;

    std::string name;

    Branch subroutineBranch;

    // Code
    InitializeFunc initialize;
    EvaluateFunc evaluate;

    bool meta;
    Term* feedbackAccumulationFunction;
    Term* feedbackPropogateFunction;

    Function();
    ~Function();

    // Hosted functions
    static void alloc(Term* caller);
    static void dealloc(Term* caller);
    static void duplicate(Term* source, Term* dest);
    static void subroutine_create(Term* caller);
    static void call_subroutine__initialize(Term* caller);
    static void call_subroutine(Term* caller);
    static void name_input(Term* caller);
    static void get_input_name(Term* caller);
};

bool is_function(Term* term);
Function& as_function(Term*);

void initialize_functions(Branch* kernel);

} // namespace circa

#endif
