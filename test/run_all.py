#!/usr/bin/env python

import os,sys,test_helper

TESTS_RUN = 0
FAILURES = []

def do_output_test(*args):
    import output_test
    global TESTS_RUN
    global FAILURES
    
    result = output_test.run_test(*args)
    if result is not True:
        FAILURES.append(result)

    TESTS_RUN += 1

def print_results():
    output = "Ran %d tests." % TESTS_RUN

    if not FAILURES:
        output += " All tests passed."
        print output
    else:
        output += " %d tests failed:" % len(FAILURES)
        print output

        for failure in FAILURES:
            print failure

for file in test_helper.readFileAsLines('test_manifest'):
    do_output_test(file)

print_results()
