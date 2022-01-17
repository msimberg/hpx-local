//  Copyright (c) 2021 ETH Zurich
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <pika/modules/execution.hpp>
#include <pika/modules/testing.hpp>

#include "algorithm_test_utils.hpp"

#include <atomic>
#include <exception>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

namespace ex = pika::execution::experimental;

// This overload is only used to check dispatching. It is not a useful
// implementation.
void tag_invoke(ex::start_detached_t, custom_sender2 s)
{
    s.tag_invoke_overload_called = true;
}

int main()
{
    {
        std::atomic<bool> start_called{false};
        std::atomic<bool> connect_called{false};
        std::atomic<bool> tag_invoke_overload_called{false};
        ex::start_detached(custom_sender{
            start_called, connect_called, tag_invoke_overload_called});
        PIKA_TEST(start_called);
        PIKA_TEST(connect_called);
        PIKA_TEST(!tag_invoke_overload_called);
    }

    {
        std::atomic<bool> start_called{false};
        std::atomic<bool> connect_called{false};
        std::atomic<bool> tag_invoke_overload_called{false};
        ex::start_detached(custom_typed_sender<int>{
            0, start_called, connect_called, tag_invoke_overload_called});
        PIKA_TEST(start_called);
        PIKA_TEST(connect_called);
        PIKA_TEST(!tag_invoke_overload_called);
    }

    {
        std::atomic<bool> start_called{false};
        std::atomic<bool> connect_called{false};
        std::atomic<bool> tag_invoke_overload_called{false};
        ex::start_detached(
            custom_typed_sender<custom_type_non_default_constructible>{
                custom_type_non_default_constructible{0}, start_called,
                connect_called, tag_invoke_overload_called});
        PIKA_TEST(start_called);
        PIKA_TEST(connect_called);
        PIKA_TEST(!tag_invoke_overload_called);
    }

    {
        std::atomic<bool> start_called{false};
        std::atomic<bool> connect_called{false};
        std::atomic<bool> tag_invoke_overload_called{false};
        ex::start_detached(custom_typed_sender<
            custom_type_non_default_constructible_non_copyable>{
            custom_type_non_default_constructible_non_copyable{0}, start_called,
            connect_called, tag_invoke_overload_called});
        PIKA_TEST(start_called);
        PIKA_TEST(connect_called);
        PIKA_TEST(!tag_invoke_overload_called);
    }

    // operator| overload
    {
        std::atomic<bool> start_called{false};
        std::atomic<bool> connect_called{false};
        std::atomic<bool> tag_invoke_overload_called{false};
        custom_sender{
            start_called, connect_called, tag_invoke_overload_called} |
            ex::start_detached();
        PIKA_TEST(start_called);
        PIKA_TEST(connect_called);
        PIKA_TEST(!tag_invoke_overload_called);
    }

    // tag_invoke overload
    {
        std::atomic<bool> start_called{false};
        std::atomic<bool> connect_called{false};
        std::atomic<bool> tag_invoke_overload_called{false};
        ex::start_detached(custom_sender2{custom_sender{
            start_called, connect_called, tag_invoke_overload_called}});
        PIKA_TEST(!start_called);
        PIKA_TEST(!connect_called);
        PIKA_TEST(tag_invoke_overload_called);
    }

    return pika::util::report_errors();
}
