//  Copyright (c) 2017 Igor Krivenko
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <pika/local/init.hpp>
#include <pika/program_options.hpp>

#include <iostream>
#include <string>
#include <vector>

using namespace pika::program_options;

int pika_main(variables_map& vm)
{
    std::cout << "Value of reqopt1: " << vm["reqopt1"].as<int>() << std::endl;
    std::cout << "Value of reqopt2: " << vm["reqopt2"].as<double>()
              << std::endl;
    std::cout << "Value of reqopt3: " << vm["reqopt3"].as<std::string>()
              << std::endl;

    return pika::local::finalize();
}

int main(int argc, char* argv[])
{
    std::vector<std::string> cfg = {"pika.commandline.rethrow_errors!=1"};

    char help_option[] = "--pika:help";

    std::vector<char*> newargv;
    for (int i = 0; i != argc; ++i)
    {
        newargv.push_back(argv[i]);
    }
    newargv.push_back(help_option);
    newargv.push_back(nullptr);

    options_description cmdline("Issue #2990\n\nUsage: issue2990 [options]");

    // clang-format off
    cmdline.add_options()
        ("reqopt1", value<int>()->required(), "Required option 1")
        ("reqopt2", value<double>()->required(), "Required option 2")
        ("reqopt3", value<std::string>()->required(), "Required option 3");
    // clang-format on

    pika::local::init_params init_args;
    init_args.desc_cmdline = cmdline;
    init_args.cfg = cfg;

    return pika::local::init(pika_main, argc + 1, newargv.data(), init_args);
}
