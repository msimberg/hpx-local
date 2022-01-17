//  Copyright (c) 2017-2021 Hartmut Kaiser
//  Copyright (c) 2017 Google
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <pika/local/config.hpp>
#include <pika/concepts/concepts.hpp>
#include <pika/concepts/has_member_xxx.hpp>
#include <pika/execution/detail/execution_parameter_callbacks.hpp>
#include <pika/execution/traits/executor_traits.hpp>
#include <pika/execution_base/execution.hpp>
#include <pika/execution_base/traits/is_executor.hpp>
#include <pika/functional/detail/tag_fallback_invoke.hpp>
#include <pika/modules/topology.hpp>

#include <cstddef>
#include <type_traits>
#include <utility>

namespace pika { namespace parallel { namespace execution {

    ///////////////////////////////////////////////////////////////////////////
    // define member traits
    PIKA_HAS_MEMBER_XXX_TRAIT_DEF(has_pending_closures)
    PIKA_HAS_MEMBER_XXX_TRAIT_DEF(get_pu_mask)
    PIKA_HAS_MEMBER_XXX_TRAIT_DEF(set_scheduler_mode)

    ///////////////////////////////////////////////////////////////////////////
    // define customization points

    /// Retrieve whether this executor has operations pending or not.
    ///
    /// \param exec  [in] The executor object to use to extract the
    ///              requested information for.
    ///
    /// \note If the executor does not expose this information, this call
    ///       will always return \a false
    ///
    inline constexpr struct has_pending_closures_t final
      : pika::functional::detail::tag_fallback<has_pending_closures_t>
    {
    private:
        // clang-format off
        template <typename Executor,
            PIKA_CONCEPT_REQUIRES_(
                pika::traits::is_executor_any<Executor>::value
            )>
        // clang-format on
        friend PIKA_FORCEINLINE decltype(auto) tag_fallback_invoke(
            has_pending_closures_t, Executor&& /*exec*/)
        {
            return false;    // assume stateless scheduling
        }

        // clang-format off
        template <typename Executor,
            PIKA_CONCEPT_REQUIRES_(
                pika::traits::is_executor_any<Executor>::value &&
                has_has_pending_closures<Executor>::value
            )>
        // clang-format on
        friend PIKA_FORCEINLINE decltype(auto) tag_invoke(
            has_pending_closures_t, Executor&& exec)
        {
            return exec.has_pending_closures();
        }
    } has_pending_closures{};

    /// Retrieve the bitmask describing the processing units the given
    /// thread is allowed to run on
    ///
    /// All threads::executors invoke sched.get_pu_mask().
    ///
    /// \param exec  [in] The executor object to use for querying the
    ///              number of pending tasks.
    /// \param topo  [in] The topology object to use to extract the
    ///              requested information.
    /// \param thream_num [in] The sequence number of the thread to
    ///              retrieve information for.
    ///
    /// \note If the executor does not support this operation, this call
    ///       will always invoke pika::threads::get_pu_mask()
    ///
    inline constexpr struct get_pu_mask_t final
      : pika::functional::detail::tag_fallback<get_pu_mask_t>
    {
    private:
        // clang-format off
        template <typename Executor,
            PIKA_CONCEPT_REQUIRES_(
                pika::traits::is_executor_any<Executor>::value
            )>
        // clang-format on
        friend PIKA_FORCEINLINE decltype(auto) tag_fallback_invoke(get_pu_mask_t,
            Executor&& /*exec*/, threads::topology& topo,
            std::size_t thread_num)
        {
            return detail::get_pu_mask(topo, thread_num);
        }

        // clang-format off
        template <typename Executor,
            PIKA_CONCEPT_REQUIRES_(
                pika::traits::is_executor_any<Executor>::value &&
                has_get_pu_mask<Executor>::value
            )>
        // clang-format on
        friend PIKA_FORCEINLINE decltype(auto) tag_invoke(get_pu_mask_t,
            Executor&& exec, threads::topology& topo, std::size_t thread_num)
        {
            return exec.get_pu_mask(topo, thread_num);
        }
    } get_pu_mask{};

    /// Set various modes of operation on the scheduler underneath the
    /// given executor.
    ///
    /// \param exec     [in] The executor object to use.
    /// \param mode     [in] The new mode for the scheduler to pick up
    ///
    /// \note This calls exec.set_scheduler_mode(mode) if it exists;
    ///       otherwise it does nothing.
    ///
    inline constexpr struct set_scheduler_mode_t final
      : pika::functional::detail::tag_fallback<set_scheduler_mode_t>
    {
    private:
        // clang-format off
        template <typename Executor, typename Mode,
            PIKA_CONCEPT_REQUIRES_(
                pika::traits::is_executor_any<Executor>::value
            )>
        // clang-format on
        friend PIKA_FORCEINLINE void tag_fallback_invoke(
            set_scheduler_mode_t, Executor&& /*exec*/, Mode const& /*mode*/)
        {
        }

        // clang-format off
        template <typename Executor, typename Mode,
            PIKA_CONCEPT_REQUIRES_(
                pika::traits::is_executor_any<Executor>::value &&
                has_set_scheduler_mode<Executor>::value
            )>
        // clang-format on
        friend PIKA_FORCEINLINE void tag_invoke(
            set_scheduler_mode_t, Executor&& exec, Mode const& mode)
        {
            exec.set_scheduler_mode(mode);
        }
    } set_scheduler_mode{};
}}}    // namespace pika::parallel::execution
