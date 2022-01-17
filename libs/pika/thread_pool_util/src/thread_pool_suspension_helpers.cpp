//  Copyright (c) 2019 Mikael Simberg
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <pika/async_local/apply.hpp>
#include <pika/async_local/async.hpp>
#include <pika/functional/function.hpp>
#include <pika/futures/future.hpp>
#include <pika/thread_pool_util/thread_pool_suspension_helpers.hpp>
#include <pika/threading_base/scheduler_base.hpp>
#include <pika/threading_base/thread_data.hpp>
#include <pika/threading_base/thread_pool_base.hpp>

#include <cstddef>
#include <utility>

namespace pika { namespace threads {
    pika::future<void> resume_processing_unit(
        thread_pool_base& pool, std::size_t virt_core)
    {
        if (!threads::get_self_ptr())
        {
            PIKA_THROW_EXCEPTION(invalid_status, "resume_processing_unit",
                "cannot call resume_processing_unit from outside pika, use"
                "resume_processing_unit_cb instead");
        }
        else if (!pool.get_scheduler()->has_scheduler_mode(
                     policies::enable_elasticity))
        {
            return pika::make_exceptional_future<void>(
                PIKA_GET_EXCEPTION(invalid_status, "resume_processing_unit",
                    "this thread pool does not support suspending "
                    "processing units"));
        }

        return pika::async([&pool, virt_core]() -> void {
            return pool.resume_processing_unit_direct(virt_core, throws);
        });
    }

    void resume_processing_unit_cb(thread_pool_base& pool,
        util::function_nonser<void(void)> callback, std::size_t virt_core,
        error_code& ec)
    {
        if (!pool.get_scheduler()->has_scheduler_mode(
                policies::enable_elasticity))
        {
            PIKA_THROWS_IF(ec, invalid_status, "resume_processing_unit_cb",
                "this thread pool does not support suspending "
                "processing units");
            return;
        }

        auto resume_direct_wrapper = [&pool, virt_core,
                                         callback = PIKA_MOVE(callback)]() {
            pool.resume_processing_unit_direct(virt_core, throws);
            callback();
        };

        if (threads::get_self_ptr())
        {
            pika::apply(PIKA_MOVE(resume_direct_wrapper));
        }
        else
        {
            std::thread(PIKA_MOVE(resume_direct_wrapper)).detach();
        }
    }

    pika::future<void> suspend_processing_unit(
        thread_pool_base& pool, std::size_t virt_core)
    {
        if (!threads::get_self_ptr())
        {
            PIKA_THROW_EXCEPTION(invalid_status, "suspend_processing_unit",
                "cannot call suspend_processing_unit from outside pika, use"
                "suspend_processing_unit_cb instead");
        }
        if (!pool.get_scheduler()->has_scheduler_mode(
                policies::enable_elasticity))
        {
            return pika::make_exceptional_future<void>(
                PIKA_GET_EXCEPTION(invalid_status, "suspend_processing_unit",
                    "this thread pool does not support suspending "
                    "processing units"));
        }
        if (!pool.get_scheduler()->has_scheduler_mode(
                policies::enable_stealing) &&
            pika::this_thread::get_pool() == &pool)
        {
            return pika::make_exceptional_future<void>(
                PIKA_GET_EXCEPTION(invalid_status, "suspend_processing_unit",
                    "this thread pool does not support suspending "
                    "processing units from itself (no thread stealing)"));
        }

        return pika::async([&pool, virt_core]() -> void {
            return pool.suspend_processing_unit_direct(virt_core, throws);
        });
    }

    void suspend_processing_unit_cb(thread_pool_base& pool,
        util::function_nonser<void(void)> callback, std::size_t virt_core,
        error_code& ec)
    {
        if (!pool.get_scheduler()->has_scheduler_mode(
                policies::enable_elasticity))
        {
            PIKA_THROWS_IF(ec, invalid_status, "suspend_processing_unit_cb",
                "this thread pool does not support suspending processing "
                "units");
            return;
        }

        auto suspend_direct_wrapper = [&pool, virt_core,
                                          callback = PIKA_MOVE(callback)]() {
            pool.suspend_processing_unit_direct(virt_core, throws);
            callback();
        };

        if (threads::get_self_ptr())
        {
            if (!pool.get_scheduler()->has_scheduler_mode(
                    policies::enable_stealing) &&
                pika::this_thread::get_pool() == &pool)
            {
                PIKA_THROW_EXCEPTION(invalid_status,
                    "suspend_processing_unit_"
                    "cb",
                    "this thread pool does not support suspending "
                    "processing units from itself (no thread stealing)");
            }

            pika::apply(PIKA_MOVE(suspend_direct_wrapper));
        }
        else
        {
            std::thread(PIKA_MOVE(suspend_direct_wrapper)).detach();
        }
    }

    future<void> resume_pool(thread_pool_base& pool)
    {
        if (!threads::get_self_ptr())
        {
            PIKA_THROW_EXCEPTION(invalid_status, "resume_pool",
                "cannot call resume_pool from outside pika, use resume_pool_cb "
                "or the member function resume_direct instead");
            return pika::make_ready_future();
        }

        return pika::async(
            [&pool]() -> void { return pool.resume_direct(throws); });
    }

    void resume_pool_cb(thread_pool_base& pool,
        util::function_nonser<void(void)> callback, error_code& /* ec */)
    {
        auto resume_direct_wrapper =
            [&pool, callback = PIKA_MOVE(callback)]() -> void {
            pool.resume_direct(throws);
            callback();
        };

        if (threads::get_self_ptr())
        {
            pika::apply(PIKA_MOVE(resume_direct_wrapper));
        }
        else
        {
            std::thread(PIKA_MOVE(resume_direct_wrapper)).detach();
        }
    }

    future<void> suspend_pool(thread_pool_base& pool)
    {
        if (!threads::get_self_ptr())
        {
            PIKA_THROW_EXCEPTION(invalid_status, "suspend_pool",
                "cannot call suspend_pool from outside pika, use "
                "suspend_pool_cb or the member function suspend_direct "
                "instead");
            return pika::make_ready_future();
        }
        if (threads::get_self_ptr() && pika::this_thread::get_pool() == &pool)
        {
            return pika::make_exceptional_future<void>(
                PIKA_GET_EXCEPTION(bad_parameter, "suspend_pool",
                    "cannot suspend a pool from itself"));
        }

        return pika::async(
            [&pool]() -> void { return pool.suspend_direct(throws); });
    }

    void suspend_pool_cb(thread_pool_base& pool,
        util::function_nonser<void(void)> callback, error_code& ec)
    {
        if (threads::get_self_ptr() && pika::this_thread::get_pool() == &pool)
        {
            PIKA_THROWS_IF(ec, bad_parameter, "suspend_pool_cb",
                "cannot suspend a pool from itself");
            return;
        }

        auto suspend_direct_wrapper = [&pool, callback = PIKA_MOVE(callback)]() {
            pool.suspend_direct(throws);
            callback();
        };

        if (threads::get_self_ptr())
        {
            pika::apply(PIKA_MOVE(suspend_direct_wrapper));
        }
        else
        {
            std::thread(PIKA_MOVE(suspend_direct_wrapper)).detach();
        }
    }

}}    // namespace pika::threads
