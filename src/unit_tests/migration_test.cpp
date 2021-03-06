// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "block.h"
#include "building.h"
#include "hashtable.h"
#include "interpreter.h"
#include "kernel.h"
#include "migration.h"
#include "modules.h"
#include "reflection.h"
#include "world.h"

namespace migration_test {

void translate_terms()
{
    Block block1;
    Block block2;

    block1.compile("a = 1");
    Term* b1 = block1.compile("b = 2");
    block1.compile("c = 3");

    block2.compile("a = 1");
    Term* b2 = block2.compile("b = 2");
    block2.compile("c = 3");

    test_assert(b2 == migrate_term_pointer(b1, &block1, &block2));
}

void update_references()
{
    Block version1;
    version1.compile("def f()");
    Block version2;
    version2.compile("def f()");

    Block target;
    Term* call = target.compile("f()");
    change_function(call, version1.get("f"));
    test_assert(call->function == version1.get("f"));

    migrate_block(&target, &version1, &version2);

    test_assert(call->function != version1.get("f"));
    test_assert(call->function == version2.get("f"));
}

void stack_migration_deletes_block()
{
    Block block;
    block.compile("a = 1 + 2");

    Stack stack;
    stack_init(&stack, &block);

    // This once crashed:
    migrate_stack(&stack, &block, NULL);
}

void register_tests()
{
    REGISTER_TEST_CASE(migration_test::translate_terms);
    REGISTER_TEST_CASE(migration_test::update_references);
    REGISTER_TEST_CASE(migration_test::stack_migration_deletes_block);
}

}
