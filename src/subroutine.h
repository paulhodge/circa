#ifndef SUBROUTINE_INCLUDED
#define SUBROUTINE_INCLUDED

#include "common_headers.h"

#include "function.h"

namespace circa {

struct Subroutine : public Function
{
    Branch branch;
};

bool is_subroutine(Term*);
Subroutine& as_subroutine(Term*);

Branch* Subroutine_openBranch(Term* caller);
void Subroutine_closeBranch(Term* caller);
void Subroutine_evaluate(Term* caller);

void initialize_subroutine(Branch* kernel);

} // namespace circa

#endif
