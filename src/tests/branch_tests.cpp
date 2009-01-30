// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "debug.h"
#include "introspection.h"
#include "parser.h"
#include "runtime.h"
#include "term.h"
#include "testing.h"
#include "type.h"
#include "values.h"

namespace circa {
namespace branch_tests {

void test_duplicate()
{
    Branch original;
    Term* term1 = apply_function(&original, VAR_INT, ReferenceList());
    Term* term2 = apply_function(&original, VAR_STRING, ReferenceList());
    as_int(term1) = 5;
    as_string(term2) = "yarn";
    original.bindName(term1, "term1");
    original.bindName(term2, "term two");

    Branch duplicate;

    duplicate_branch(original, duplicate);

    Term* term1_duplicate = duplicate.getNamed("term1");
    test_assert(term1_duplicate != NULL);

    Term* term2_duplicate = duplicate.getNamed("term two");

    test_assert(as_int(term1_duplicate) == 5);
    test_assert(as_string(term2_duplicate) == "yarn");

    // make sure 'duplicate' uses different terms
    as_int(term1) = 8;
    test_assert(as_int(term1_duplicate) == 5);

    sanity_check_term(term1);
    sanity_check_term(term2);
    sanity_check_term(term1_duplicate);
    sanity_check_term(term2_duplicate);
}

void find_name_in_outer_branch()
{
    Branch branch;
    Term* outer_branch = branch.eval("Branch()");
    alloc_value(outer_branch);

    Term* a = as_branch(outer_branch).eval("a = 1");

    Term* inner_branch = as_branch(outer_branch).eval("Branch()");
    alloc_value(inner_branch);

    as_branch(inner_branch).outerScope = &as_branch(outer_branch);

    test_assert(find_named(&as_branch(inner_branch), "a") == a);
}

void cleanup_users_after_delete()
{
    Branch branch;

    Term* a = branch.eval("a = 1.0");

    Branch &subBranch = as_branch(branch.eval("sub = Branch()"));
    subBranch.outerScope = &branch;

    Term* b = subBranch.eval("b = add(a,a)");

    test_assert(a->users.contains(b));

    dealloc_value(branch["sub"]);

    test_assert(!a->users.contains(b));
}

void test_startBranch()
{
    Branch branch;

    Term* a = branch.eval("a = 1");

    Branch& sub = branch.startBranch("sub");

    test_assert(find_named(&sub, "a") == a);
}

void test_migrate()
{
    Branch orig, replace;

    Term* a = orig.eval("a = 1");

    replace.eval("a = 2");

    migrate_branch(replace, orig);

    // Test that the 'orig' terms are the same terms
    test_assert(orig["a"] == a);

    // Test that the value was transferred
    test_assert(as_int(a) == 2);
}

void register_tests()
{
    REGISTER_TEST_CASE(branch_tests::test_duplicate);
    REGISTER_TEST_CASE(branch_tests::find_name_in_outer_branch);
#if TRACK_USERS
    REGISTER_TEST_CASE(branch_tests::cleanup_users_after_delete);
#endif
    REGISTER_TEST_CASE(branch_tests::test_startBranch);
    REGISTER_TEST_CASE(branch_tests::test_migrate);
}

} // namespace branch_tests

} // namespace circa
