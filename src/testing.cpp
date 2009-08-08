// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "common_headers.h"

#include "circa.h"

namespace circa {

std::vector<TestCase> gTestCases;

TestCase gCurrentTestCase;

// this function is defined in register_all_tests.cpp
void register_all_tests();

void post_test_sanity_check();

void _test_assert_function(bool condition, int line, const char* file)
{
    if (!condition) {
        std::stringstream msg;
        msg << "Assert failure in " << file << ", line " << line;
        throw std::runtime_error(msg.str());
    }
}

void _test_assert_function(Term* term, int line, const char* file)
{
    if (term == NULL) {
        std::stringstream msg;
        msg << "NULL term in " << file << ", line " << line;
        throw std::runtime_error(msg.str());
    }

    if (term->hasError) {
        std::stringstream msg;
        msg << "Runtime error on term " << format_global_id(term) << std::endl;
        msg << term->getErrorMessage() << std::endl;
        msg << "Occurred in " << file << ", line " << line << std::endl;
        throw std::runtime_error(msg.str());
    }

    if (has_static_error(term)) {
        std::stringstream msg;
        msg << "Compile error on term " << format_global_id(term) << std::endl;
        msg << get_static_error_message(term) << std::endl;
        msg << "Occurred in " << file << ", line " << line << std::endl;
        throw std::runtime_error(msg.str());
    }
}

void _test_assert_function(Branch& branch, int line, const char* file)
{
    // Sanity check on every term in this branch
    // Note that currently, we don't have many sanity checks. It would be nice
    // to add some more.
    for (BranchIterator it(branch); !it.finished(); ++it) {
        Term* term = *it;

        bool result = true;
        std::string message;

        if (is_subroutine(term))
            result = function_t::check_invariants(term, NULL);

        if (!result) {
            std::stringstream msg;
            msg << "Sanity check fail at " << file << ", line " << line << std::endl;
            msg << message << std::endl;
            throw std::runtime_error(msg.str());
        }
    }

    if (has_static_errors(branch)) {
        std::stringstream msg;
        msg << "Branch has errors at " << file << ", line " << line << std::endl;
        print_static_errors_formatted(branch, msg);
        throw std::runtime_error(msg.str());
    }
}

void _test_fail_function(int line, const char* file)
{
    std::stringstream msg;
    msg << "Test fail in " << file << ", line " << line;
    throw std::runtime_error(msg.str());
}

void _test_equals_function(RefList const& a, RefList const& b,
        const char* aText, const char* bText, int line, const char* file)
{
    std::stringstream msg;

    if (a.length() != b.length()) {
        msg << "List equality fail in " << file << ", line " << line << std::endl;
        msg << "  " << aText << " has " << a.length() << " items, ";
        msg << bText << " has " << b.length() << " items.";
        throw std::runtime_error(msg.str());
    }

    for (int i=0; i < a.length(); i++) {
        if (a[i] != b[i]) {
            msg << "List equality fail in " << file << ", line " << line << std::endl;
            msg << "  " << aText << " != " << bText << " (index " << i << " differs)";
            throw std::runtime_error(msg.str());
        }
    }
}

void _test_equals_function(float a, float b,
        const char* aText, const char* bText,
        int line, const char* file)
{
    const float EPSILON = 0.0001f;

    if (fabs(a-b) > EPSILON) {
        std::stringstream msg;
        msg << "Equality fail in " << file << ", line " << line << std::endl;
        msg << aText << " [" << a << "] != " << bText << " [" << b << "]";
        throw std::runtime_error(msg.str());
    }
}

void _test_equals_function(std::string a, std::string b,
        const char* aText, const char* bText,
        int line, const char* file)
{
    if (a != b) {
        std::stringstream msg;
        msg << "Failed assert: " << a << " != " << b;
        msg << ", in " << file << ", line " << line << std::endl;
        throw std::runtime_error(msg.str());
    }
}

bool run_test(TestCase& testCase, bool catch_exceptions)
{
    gCurrentTestCase = testCase;

    if (catch_exceptions) {
        try {
            gCurrentTestCase = testCase;
            testCase.execute();

            // the test code may declare itself failed
            bool result = !gCurrentTestCase.failed;

            post_test_sanity_check();
            return result;
        }
        catch (std::runtime_error const& err) {
            std::cout << "Error white running test case " << testCase.name << std::endl;
            std::cout << err.what() << std::endl;
            return false;
        }
    } else {
        testCase.execute();
    }

    post_test_sanity_check();

    return !gCurrentTestCase.failed;
}

void run_test_named(std::string const& testName)
{
    register_all_tests();

    std::vector<TestCase>::iterator it;
    for (it = gTestCases.begin(); it != gTestCases.end(); ++it) {
        if (it->name == testName) {
            bool result = run_test(*it, true);
            if (result)
                std::cout << testName << " succeeded" << std::endl;
            return;
        }
    }

    std::cout << "Couldn't find a test named: " << testName << std::endl;
}

void run_all_tests()
{
    register_all_tests();

    int totalTestCount = (int) gTestCases.size();
    int successCount = 0;
    int failureCount = 0;
    std::vector<TestCase>::iterator it;
    for (it = gTestCases.begin(); it != gTestCases.end(); ++it) {
        bool result = run_test(*it, false);
        if (result) successCount++;
        else failureCount++;
    }

    std::cout << "Ran " << totalTestCount << " tests. ";

    if (failureCount == 0) {
        std::cout << "All tests passed." << std::endl;
    } else {
        std::string tests = failureCount == 1 ? "test" : "tests";
        std::cout << failureCount << " " << tests << " failed." << std::endl;
    }
}

void post_test_sanity_check()
{
    sanity_check_the_world();
}

std::vector<std::string> list_all_test_names()
{
    register_all_tests();

    std::vector<std::string> output;

    std::vector<TestCase>::iterator it;
    for (it = gTestCases.begin(); it != gTestCases.end(); ++it)
        output.push_back(it->name);

    return output;
}

std::string get_current_test_name()
{
    return gCurrentTestCase.name;
}

void declare_current_test_failed()
{
    gCurrentTestCase.failed = true;
}

}
