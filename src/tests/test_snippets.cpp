// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

// In these tests, we have a bunch of "snippets" (small pieces of code). We execute
// the code and then check that a list of assertions are true.
//
// Each test should call the function test_snippet(code, assertions). Both inputs
// are strings which are compiled as Circa code. We compile 'code', then we compile
// 'assertions' as a subbranch in the 'code' branch. If there are any static errors,
// then the test fails. Then we go through the 'assertions' branch, and we look
// for any statements which return a boolean, and we make sure all of those have
// evaluated to true. (if any are false, the test fails)

#include "common_headers.h"

#include <circa.h>

namespace circa {
namespace test_snippets {

void test_snippet(std::string codeStr, std::string assertionsStr)
{
    Branch code;
    parser::compile(&code, parser::statement_list, codeStr);

    if (has_static_errors(code)) {
        std::cout << "In code snippet: " << codeStr;
        print_static_errors_formatted(code, std::cout);
        declare_current_test_failed();
        return;
    }

    Branch& assertions = create_branch(code, "assertions");
    parser::compile(&assertions, parser::statement_list, assertionsStr);

    evaluate_branch(code);

    int boolean_statements_found = 0;
    for (int i=0; i < assertions.length(); i++) {
        if (!is_statement(assertions[i]))
            continue;

        if (!is_bool(assertions[i]))
            continue;

        boolean_statements_found++;

        if (!as_bool(assertions[i])) {
            std::cout << "In " << get_current_test_name() << std::endl;
            std::cout << "assertion failed: " << get_term_source(assertions[i]) << std::endl;
            std::cout << "Compiled code: " << std::endl;
            std::cout << branch_to_string_raw(code);
            declare_current_test_failed();
            return;
        }
    }

    if (boolean_statements_found == 0) {
        std::cout << "In " << get_current_test_name() << std::endl;
        std::cout << "no boolean statements found in: " << assertionsStr << std::endl;
        declare_current_test_failed();
    }
}

void equals_snippets()
{
    test_snippet("", "4 == 4");
    test_snippet("", "3 != 4");
    test_snippet("", "4.0 == 4.0");
    test_snippet("", "4.0 != 5.0");
    test_snippet("", "'hello' == 'hello'");
    test_snippet("", "'hello' != 'bye'");
    test_snippet("", "true == true");
    test_snippet("", "false != true");
    test_snippet("", "[1 1] == [1 1]");
    test_snippet("", "[1 1] != [1 2]");
    test_snippet("", "[1 1 1] != [1 1]");
}

void register_tests()
{
    REGISTER_TEST_CASE(test_snippets::equals_snippets);
}

} // namespace test_snippets
} // namespace circa
