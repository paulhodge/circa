// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace list_function {

    void evaluate(Term* caller) {
        Branch& dest = as_branch(caller);

        int numTermsToAssign = std::min(caller->numInputs(), dest.numTerms());

        for (int i=0; i < numTermsToAssign; i++) {
            assign_value(caller->input(i), dest[i]);
        }

        // Add terms if necessary
        for (int i=dest.numTerms(); i < caller->numInputs(); i++) {
            create_duplicate(&dest, caller->input(i));
        }

        // Remove terms if necessary
        for (int i=caller->numInputs(); i < dest.numTerms(); i++) {
            dest[i] = NULL;
        }
        dest.removeNulls();
    }

    std::string toSourceString(Term* caller) {
        std::stringstream out;
        if (caller->name != "")
            out << caller->name << " = ";
        out << "[";
        for (int i=0; i < caller->numInputs(); i++)
            out << get_source_of_input(caller, i);
        out << "]";
        return out.str();
    }

    void setup(Branch& kernel)
    {
        LIST_FUNC = import_function(kernel, evaluate, "list(any...) : List");
        as_function(LIST_FUNC).pureFunction = true;
        as_function(LIST_FUNC).toSourceString = toSourceString;
    }
}
}
