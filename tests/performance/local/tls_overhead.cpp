//  Copyright (c) 2011 Bryce Adelstein-Lelbach
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <pika/local/config.hpp>

#include <pika/modules/format.hpp>
#include <pika/thread_support/thread_specific_ptr.hpp>
#include <pika/modules/timing.hpp>

#include <boost/config.hpp>
#include <pika/modules/program_options.hpp>

#include <thread>
#include <cstdint>
#include <iostream>
#include <vector>

using pika::program_options::variables_map;
using pika::program_options::options_description;
using pika::program_options::value;
using pika::program_options::store;
using pika::program_options::command_line_parser;
using pika::program_options::notify;

using pika::chrono::high_resolution_timer;
using pika::util::thread_specific_ptr;

///////////////////////////////////////////////////////////////////////////////
// thread local globals
struct tag { };
static thread_specific_ptr<double, tag> global_scratch;

///////////////////////////////////////////////////////////////////////////////
inline void worker(
    std::uint64_t updates
    )
{
    global_scratch.reset(new double);

    for (double i = 0.; i < updates; ++i)
        *global_scratch += 1. / (2. * i + 1.);

    global_scratch.reset();
}

///////////////////////////////////////////////////////////////////////////////
int main(
    int argc
  , char** argv
    )
{
    ///////////////////////////////////////////////////////////////////////////
    // parse command line
    variables_map vm;

    options_description cmdline("Usage: " PIKA_APPLICATION_STRING " [options]");

    std::uint64_t threads, updates;

    cmdline.add_options()
        ( "help,h"
        , "print out program usage (this message)")

        ( "threads,t"
        , value<std::uint64_t>(&threads)->default_value(1),
         "number of OS-threads")

        ( "updates,u"
        , value<std::uint64_t>(&updates)->default_value(1 << 22)
        , "updates made to the TLS variable per OS-thread")

        ( "csv"
        , "output results as csv (format: updates,OS-threads,duration)")
        ;
    ;

    store(command_line_parser(argc, argv).options(cmdline).run(), vm);

    notify(vm);

    ///////////////////////////////////////////////////////////////////////////
    // print help screen
    if (vm.count("help"))
    {
        std::cout << cmdline;
        return 0;
    }

    ///////////////////////////////////////////////////////////////////////////
    // run the test
    std::vector<std::thread> workers;

    high_resolution_timer t;

    for (std::uint64_t i = 0; i != threads; ++i)
        workers.push_back(std::thread(worker, updates));

    for (std::thread& thread : workers)
    {
        if (thread.joinable())
            thread.join();
    }

    const double duration = t.elapsed();

    ///////////////////////////////////////////////////////////////////////////
    // output results
    if (vm.count("csv"))
        pika::util::format_to(std::cout,
            "{1},{2},{3}\n",
            updates,
            threads,
            duration);
    else
        pika::util::format_to(std::cout,
            "ran {1} updates per OS-thread on {2} "
            "OS-threads in {3} seconds\n",
            updates,
            threads,
            duration);
}

