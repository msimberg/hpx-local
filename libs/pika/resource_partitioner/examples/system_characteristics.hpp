//  Copyright (c) 2017 Shoshana Jakobovits
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <pika/local/runtime.hpp>
#include <pika/modules/threadmanager.hpp>

#include <iostream>

void print_system_characteristics()
{
    std::cout << "[pika_main] System queries: \n\n";

    // -------------------------------------- //
    //      get pointer to instances          //
    //      I can query                       //
    // -------------------------------------- //

    pika::runtime* rt = pika::get_runtime_ptr();
    pika::util::runtime_configuration cfg = rt->get_config();
    const pika::threads::topology& topo = rt->get_topology();

    // -------------------------------------- //
    //      print runtime characteristics     //
    //                                        //
    // -------------------------------------- //

    //! -------------------------------------- runtime
    std::cout << "[Runtime], instance number " << rt->get_instance_number()
              << "\n"
              << "called by thread named     " << pika::get_thread_name()
              << "\n\n";

    //! -------------------------------------- thread_manager
    std::cout << "[Thread manager]\n"
              << "worker thread number  : " << std::dec
              << pika::get_worker_thread_num() << "\n\n";

    //! -------------------------------------- runtime_configuration
    std::cout << "[Runtime configuration]\n"
              << "os thread count       : " << cfg.get_os_thread_count() << "\n"
              << "                        " << pika::get_os_thread_count()
              << "\n"
              << "command line          : " << cfg.get_cmd_line() << "\n\n";

    //! -------------------------------------- topology
    topo.print_hwloc(std::cout);
}
