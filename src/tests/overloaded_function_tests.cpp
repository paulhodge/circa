// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace overloaded_function_tests {

void declared_in_script()
{
    Branch branch;
    branch.compile("def f_1(int i) -> int return i + 1 end");
    branch.compile("def f_2(string s) -> string return concat(s 'x') end");
    branch.compile("f = overloaded_function(f_1 f_2)");
    test_assert(branch);

    EvalContext context;
    evaluate_branch(&context, branch);
    test_assert(context);

    TaggedValue* a = branch.eval("a = f(1)");
    test_equals(a, 2);
    TaggedValue* b = branch.eval("b = f('aaa')");
    test_equals(b, "aaax");
}

void update_output_type()
{
    Branch branch;
    Term* f1 = branch.compile("def f1() -> int return 0");
    Term* f = overloaded_function::create_overloaded_function(branch, "f", RefList(f1));
    test_assert(function_t::get_output_type(f) == INT_TYPE);
    Term* f2 = branch.compile("def f2() -> string return ''");
    overloaded_function::append_overload(f, f2);
    test_assert(function_t::get_output_type(f) != INT_TYPE);
}

void test_specialize_type()
{
    Branch branch;
    Term* t = branch.compile("add(1,2)");
    test_assert(t->type == INT_TYPE);
    Term* t2 = branch.compile("add(1.0,2.0)");
    test_assert(t2->type == FLOAT_TYPE);
}

void register_tests()
{
    REGISTER_TEST_CASE(overloaded_function_tests::declared_in_script);
    REGISTER_TEST_CASE(overloaded_function_tests::update_output_type);
    REGISTER_TEST_CASE(overloaded_function_tests::test_specialize_type);
}

} // namespace overloaded_function_tests
} // namespace circa