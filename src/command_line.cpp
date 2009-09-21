// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

namespace circa {

int run_command_line(std::vector<std::string> args)
{
    initialize();

    if (args.size() == 0) {
        run_all_tests();
        shutdown();
        return 0;
    }

    // Eval mode
    if (args[0] == "-e") {
        args.erase(args.begin());
        std::stringstream command;
        bool firstArg = true;
        while (!args.empty()) {
            if (!firstArg) command << " ";
            command << args[0];
            args.erase(args.begin());
            firstArg = false;
        }

        Branch workspace;
        Term* result = workspace.eval(command.str());
        std::cout << result->toString() << std::endl;
        shutdown();
        return 0;
    }

    if (args[0] == "--list-tests") {
        std::vector<std::string> testNames = list_all_test_names();

        std::vector<std::string>::const_iterator it;
        for (it = testNames.begin(); it != testNames.end(); ++it) {
            std::cout << *it << std::endl;
        }
        shutdown();
        return 0;
    }

    // Show compiled code
    if (args[0] == "-p") {
        Branch branch;
        parse_script(branch, args[1]);
        std::cout << branch_to_string_raw(branch);
        return 0;
    }

    // Show compiled code with properties
    if (args[0] == "-pp") {
        Branch branch;
        parse_script(branch, args[1]);
        std::cout << branch_to_string_raw_with_properties(branch);
        return 0;
    }

    // Reproduce source
    if (args[0] == "-s") {
        Branch branch;
        parse_script(branch, args[1]);
        std::cout << get_branch_source(branch) << std::endl;
        return 0;
    }

    // Do a feedback test
    if (args[0] == "-f") {
        Branch branch;
        parse_script(branch, args[1]);

        Branch &trainable_names = branch["_trainable"]->asBranch();
        for (int i=0; i < trainable_names.length(); i++)
            set_trainable(branch[trainable_names[i]->asString()], true);
        refresh_training_branch(branch);

        std::cout << std::endl;
        std::cout << "-- Before evaluation:" << std::endl;
        std::cout << branch_to_string_raw(branch);

        evaluate_branch(branch);

        std::cout << std::endl;
        std::cout << "-- After evaluation:" << std::endl;
        std::cout << branch_to_string_raw(branch);

        std::cout << std::endl;
        std::cout << "-- Code result:" << std::endl;
        std::cout << get_branch_source(branch) << std::endl;

        return 0;
    }

    // Show metrics for evaluation
    if (args[0] == "-m") {
        Branch branch;
        parse_script(branch, args[1]);

        // Do a throw-away evaluation, because some initialization happens the
        // first time through.
        evaluate_branch(branch);

        METRIC_TERMS_CREATED = 0;
        METRIC_TERMS_DESTROYED = 0;
        METRIC_NAMESPACE_LOOKUPS = 0;
        METRIC_NAMESPACE_BINDINGS = 0;

        // temp
        //DEBUG_TRAP_NAME_LOOKUP = true;

        evaluate_branch(branch);

        std::cout << "terms_created: " << METRIC_TERMS_CREATED << std::endl;
        std::cout << "terms_destroyed: " << METRIC_TERMS_CREATED << std::endl;
        std::cout << "namespace_lookups: " << METRIC_NAMESPACE_LOOKUPS << std::endl;
        std::cout << "namespace_bindings: " << METRIC_NAMESPACE_BINDINGS << std::endl;

        return 0;
    }

    // Generate cpp headers
    if (args[0] == "-gh") {
        Branch branch;
        parse_script(branch, args[1]);

        if (has_static_errors(branch)) {
            print_static_errors_formatted(branch, std::cout);
            return 1;
        }

        std::cout << generate_cpp_headers(branch);

        return 0;
    }

    // Generate docs
    /*if (args[0] == "-gd") {
        std::cout << "writing docs to " << args[1] << std::endl;
        std::stringstream out;
        generate_docs(*KERNEL, out);
        write_text_file(args[1], out.str());
        return 0;
    }*/

    // Otherwise, run script
    Branch main_branch;
    parse_script(main_branch, args[0]);

    if (has_static_errors(main_branch)) {
        print_static_errors_formatted(main_branch, std::cout);
        return 1;
    } else {

        Term* error_listener = new Term();

        evaluate_branch(main_branch, error_listener);

        if (error_listener->hasError) {
            std::cout << "Error occurred: " << error_listener->getErrorMessage() << std::endl;
            return 1;
        }
    }

    shutdown();
    return 0;
}

} // namespace circa

