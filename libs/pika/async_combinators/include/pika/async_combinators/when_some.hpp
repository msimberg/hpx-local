//  Copyright (c) 2007-2021 Hartmut Kaiser
//  Copyright (c) 2013 Agustin Berge
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/// \file when_some.hpp

#pragma once

#if defined(DOXYGEN)
namespace pika {

    ///////////////////////////////////////////////////////////////////////////
    /// Result type for \a when_some, contains a sequence of futures and
    /// indices pointing to ready futures.
    template <typename Sequence>
    struct when_some_result
    {
        /// List of indices of futures that have become ready
        std::vector<std::size_t> indices;

        /// The sequence of futures as passed to \a pika::when_some
        Sequence futures;
    };

    /// The function \a when_some is an operator allowing to join on the result
    /// of all given futures. It AND-composes all future objects given and
    /// returns a new future object representing the same list of futures
    /// after n of them finished executing.
    ///
    /// \param n        [in] The number of futures out of the arguments which
    ///                 have to become ready in order for the returned future
    ///                 to get ready.
    /// \param first    [in] The iterator pointing to the first element of a
    ///                 sequence of \a future or \a shared_future objects for
    ///                 which \a when_all should wait.
    /// \param last     [in] The iterator pointing to the last element of a
    ///                 sequence of \a future or \a shared_future objects for
    ///                 which \a when_all should wait.
    ///
    /// \note The future returned by the function \a when_some becomes ready
    ///       when at least \a n argument futures have become ready.
    ///
    /// \return   Returns a when_some_result holding the same list of futures
    ///           as has been passed to when_some and indices pointing to
    ///           ready futures.
    ///           - future<when_some_result<Container<future<R>>>>: If the input
    ///             cardinality is unknown at compile time and the futures
    ///             are all of the same type. The order of the futures in the
    ///             output container will be the same as given by the input
    ///             iterator.
    ///
    /// \note Calling this version of \a when_some where first == last, returns
    ///       a future with an empty container that is immediately ready.
    ///       Each future and shared_future is waited upon and then copied into
    ///       the collection of the output (returned) future, maintaining the
    ///       order of the futures in the input collection.
    ///       The future returned by \a when_some will not throw an exception,
    ///       but the futures held in the output collection may.
    ///
    template <typename InputIter,
        typename Container = vector<
            future<typename std::iterator_traits<InputIter>::value_type>>>
    future<when_some_result<Container>> when_some(
        std::size_t n, Iterator first, Iterator last);

    /// The function \a when_some is an operator allowing to join on the result
    /// of all given futures. It AND-composes all future objects given and
    /// returns a new future object representing the same list of futures
    /// after n of them finished executing.
    ///
    /// \param n        [in] The number of futures out of the arguments which
    ///                 have to become ready in order for the returned future
    ///                 to get ready.
    /// \param futures  [in] A container holding an arbitrary amount of \a future
    ///                 or \a shared_future objects for which \a when_some
    ///                 should wait.
    ///
    /// \note The future returned by the function \a when_some becomes ready
    ///       when at least \a n argument futures have become ready.
    ///
    /// \return   Returns a when_some_result holding the same list of futures
    ///           as has been passed to when_some and indices pointing to
    ///           ready futures.
    ///           - future<when_some_result<Container<future<R>>>>: If the input
    ///             cardinality is unknown at compile time and the futures
    ///             are all of the same type. The order of the futures in the
    ///             output container will be the same as given by the input
    ///             iterator.
    ///
    /// \note Each future and shared_future is waited upon and then copied into
    ///       the collection of the output (returned) future, maintaining the
    ///       order of the futures in the input collection.
    ///       The future returned by \a when_some will not throw an exception,
    ///       but the futures held in the output collection may.
    ///
    template <typename Range>
    future<when_some_result<Range>> when_some(std::size_t n, Range&& futures);

    /// The function \a when_some is an operator allowing to join on the result
    /// of all given futures. It AND-composes all future objects given and
    /// returns a new future object representing the same list of futures
    /// after n of them finished executing.
    ///
    /// \param n        [in] The number of futures out of the arguments which
    ///                 have to become ready in order for the returned future
    ///                 to get ready.
    /// \param futures  [in] An arbitrary number of \a future or \a shared_future
    ///                 objects, possibly holding different types for which
    ///                 \a when_some should wait.
    ///
    /// \note The future returned by the function \a when_some becomes ready
    ///       when at least \a n argument futures have become ready.
    ///
    /// \return   Returns a when_some_result holding the same list of futures
    ///           as has been passed to when_some and an index pointing to a
    ///           ready future..
    ///           - future<when_some_result<tuple<future<T0>, future<T1>...>>>:
    ///             If inputs are fixed in number and are of heterogeneous
    ///             types. The inputs can be any arbitrary number of future
    ///             objects.
    ///           - future<when_some_result<tuple<>>> if \a when_some is
    ///             called with zero arguments.
    ///             The returned future will be initially ready.
    ///
    /// \note Each future and shared_future is waited upon and then copied into
    ///       the collection of the output (returned) future, maintaining the
    ///       order of the futures in the input collection.
    ///       The future returned by \a when_some will not throw an exception,
    ///       but the futures held in the output collection may.
    ///
    template <typename... Ts>
    future<when_some_result<tuple<future<T>...>>> when_some(
        std::size_t n, Ts&&... futures);

    /// The function \a when_some_n is an operator allowing to join on the result
    /// of all given futures. It AND-composes all future objects given and
    /// returns a new future object representing the same list of futures
    /// after n of them finished executing.
    ///
    /// \param n        [in] The number of futures out of the arguments which
    ///                 have to become ready in order for the returned future
    ///                 to get ready.
    /// \param first    [in] The iterator pointing to the first element of a
    ///                 sequence of \a future or \a shared_future objects for
    ///                 which \a when_all should wait.
    /// \param count    [in] The number of elements in the sequence starting at
    ///                 \a first.
    ///
    /// \note The future returned by the function \a when_some_n becomes ready
    ///       when at least \a n argument futures have become ready.
    ///
    /// \return   Returns a when_some_result holding the same list of futures
    ///           as has been passed to when_some and indices pointing to
    ///           ready futures.
    ///           - future<when_some_result<Container<future<R>>>>: If the input
    ///             cardinality is unknown at compile time and the futures
    ///             are all of the same type. The order of the futures in the
    ///             output container will be the same as given by the input
    ///             iterator.
    ///
    /// \note Calling this version of \a when_some_n where count == 0, returns
    ///       a future with the same elements as the arguments that is
    ///       immediately ready. Possibly none of the futures in that container
    ///       are ready.
    ///       Each future and shared_future is waited upon and then copied into
    ///       the collection of the output (returned) future, maintaining the
    ///       order of the futures in the input collection.
    ///       The future returned by \a when_some_n will not throw an exception,
    ///       but the futures held in the output collection may.
    ///
    template <typename InputIter,
        typename Container = vector<
            future<typename std::iterator_traits<InputIter>::value_type>>>
    future<when_some_result<Container>> when_some_n(
        std::size_t n, Iterator first, std::size_t count);
}    // namespace pika

#else    // DOXYGEN

#include <pika/local/config.hpp>
#include <pika/assert.hpp>
#include <pika/datastructures/tuple.hpp>
#include <pika/functional/deferred_call.hpp>
#include <pika/futures/future.hpp>
#include <pika/futures/futures_factory.hpp>
#include <pika/futures/traits/acquire_future.hpp>
#include <pika/futures/traits/acquire_shared_state.hpp>
#include <pika/futures/traits/detail/future_traits.hpp>
#include <pika/futures/traits/future_access.hpp>
#include <pika/futures/traits/is_future.hpp>
#include <pika/futures/traits/is_future_range.hpp>
#include <pika/modules/errors.hpp>
#include <pika/type_support/pack.hpp>
#include <pika/util/detail/reserve.hpp>

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <iterator>
#include <memory>
#include <mutex>
#include <type_traits>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace pika {

    template <typename Sequence>
    struct when_some_result
    {
        when_some_result() = default;

        explicit when_some_result(Sequence&& futures) noexcept
          : indices()
          , futures(PIKA_MOVE(futures))
        {
        }

        std::vector<std::size_t> indices;
        Sequence futures;
    };

    namespace lcos { namespace detail {

        ///////////////////////////////////////////////////////////////////////
        template <typename Sequence>
        struct when_some;

        template <typename Sequence>
        struct set_when_some_callback_impl
        {
            explicit set_when_some_callback_impl(
                when_some<Sequence>& when) noexcept
              : when_(when)
              , idx_(0)
            {
            }

            template <typename Future>
            std::enable_if_t<traits::is_future_v<Future>> operator()(
                Future& future) const
            {
                std::size_t counter =
                    when_.count_.load(std::memory_order_seq_cst);
                if (counter < when_.needed_count_)
                {
                    // handle future only if not enough futures are ready
                    // yet also, do not touch any futures which are already
                    // ready

                    auto shared_state =
                        traits::detail::get_shared_state(future);

                    if (shared_state && !shared_state->is_ready())
                    {
                        shared_state->execute_deferred();

                        // execute_deferred might have made the future ready
                        if (!shared_state->is_ready())
                        {
                            shared_state->set_on_completed(util::deferred_call(
                                &detail::when_some<Sequence>::on_future_ready,
                                when_.shared_from_this(), idx_,
                                pika::execution_base::this_thread::agent()));
                            ++idx_;

                            return;
                        }
                    }

                    {
                        using mutex_type =
                            typename detail::when_some<Sequence>::mutex_type;
                        std::lock_guard<mutex_type> l(when_.mtx_);
                        when_.values_.indices.push_back(idx_);
                    }

                    if (when_.count_.fetch_add(1) + 1 == when_.needed_count_)
                    {
                        when_.goal_reached_on_calling_thread_.store(
                            true, std::memory_order_release);
                    }
                }
                ++idx_;
            }

            template <typename Sequence_>
            PIKA_FORCEINLINE
                std::enable_if_t<traits::is_future_range_v<Sequence_>>
                operator()(Sequence_& sequence) const
            {
                apply(sequence);
            }

            template <typename Tuple, std::size_t... Is>
            PIKA_FORCEINLINE void apply(
                Tuple& tuple, util::index_pack<Is...>) const
            {
                int const _sequencer[] = {
                    (((*this)(pika::get<Is>(tuple))), 0)...};
                (void) _sequencer;
            }

            template <typename... Ts>
            PIKA_FORCEINLINE void apply(pika::tuple<Ts...>& sequence) const
            {
                apply(sequence, util::make_index_pack_t<sizeof...(Ts)>());
            }

            template <typename Sequence_>
            PIKA_FORCEINLINE void apply(Sequence_& sequence) const
            {
                std::for_each(sequence.begin(), sequence.end(), *this);
            }

            detail::when_some<Sequence>& when_;
            mutable std::size_t idx_;
        };

        template <typename Sequence>
        PIKA_FORCEINLINE void set_on_completed_callback(
            detail::when_some<Sequence>& when)
        {
            set_when_some_callback_impl<Sequence> callback(when);
            callback.apply(when.values_.futures);
        }

        template <typename Sequence>
        struct when_some
          : std::enable_shared_from_this<when_some<Sequence>>    //-V690
        {
            using mutex_type = lcos::local::spinlock;

        public:
            void on_future_ready(
                std::size_t idx, pika::execution_base::agent_ref ctx)
            {
                std::size_t const new_count = count_.fetch_add(1) + 1;
                if (new_count <= needed_count_)
                {
                    {
                        std::lock_guard<mutex_type> l(this->mtx_);
                        values_.indices.push_back(idx);
                    }

                    if (new_count == needed_count_)
                    {
                        if (ctx != pika::execution_base::this_thread::agent())
                        {
                            ctx.resume();
                        }
                        else
                        {
                            goal_reached_on_calling_thread_.store(
                                true, std::memory_order_release);
                        }
                    }
                }
            }

        private:
            when_some(when_some const&) = delete;
            when_some(when_some&&) = delete;

            when_some& operator=(when_some const&) = delete;
            when_some& operator=(when_some&&) = delete;

        public:
            using argument_type = Sequence;

            when_some(argument_type&& values, std::size_t n)
              : values_(PIKA_MOVE(values))
              , count_(0)
              , needed_count_(n)
              , goal_reached_on_calling_thread_(false)
            {
            }

            when_some_result<Sequence> operator()()
            {
                // set callback functions to executed when future is ready
                set_on_completed_callback(*this);

                // if all of the requested futures are already set, our
                // callback above has already been called often enough, otherwise
                // we suspend ourselves
                if (!goal_reached_on_calling_thread_.load(
                        std::memory_order_acquire))
                {
                    // wait for any of the futures to return to become ready
                    pika::execution_base::this_thread::suspend(
                        "pika::lcos::detail::when_some::operator()");
                }

                // at least N futures should be ready
                PIKA_ASSERT(
                    count_.load(std::memory_order_acquire) >= needed_count_);

                return PIKA_MOVE(values_);
            }

            mutable mutex_type mtx_;
            when_some_result<Sequence> values_;
            std::atomic<std::size_t> count_;
            std::size_t needed_count_;
            std::atomic<bool> goal_reached_on_calling_thread_;
        };
    }}    // namespace lcos::detail

    ///////////////////////////////////////////////////////////////////////////
    template <typename Range>
    std::enable_if_t<traits::is_future_range_v<Range>,
        pika::future<when_some_result<std::decay_t<Range>>>>
    when_some(std::size_t n, Range&& lazy_values)
    {
        using result_type = std::decay_t<Range>;

        if (n == 0)
        {
            return pika::make_ready_future(when_some_result<result_type>());
        }

        result_type values = traits::acquire_future<result_type>()(lazy_values);

        if (n > values.size())
        {
            return pika::make_exceptional_future<when_some_result<result_type>>(
                PIKA_GET_EXCEPTION(pika::bad_parameter, "pika::when_some",
                    "number of results to wait for is out of bounds"));
        }

        auto f = std::make_shared<lcos::detail::when_some<result_type>>(
            PIKA_MOVE(values), n);

        lcos::local::futures_factory<when_some_result<result_type>()> p(
            [f = PIKA_MOVE(f)]() -> when_some_result<result_type> {
                return (*f)();
            });

        auto result = p.get_future();
        p.apply();

        return result;
    }

    template <typename Iterator,
        typename Container = std::vector<
            typename lcos::detail::future_iterator_traits<Iterator>::type>,
        typename Enable =
            std::enable_if_t<pika::traits::is_iterator_v<Iterator>>>
    pika::future<when_some_result<Container>> when_some(
        std::size_t n, Iterator begin, Iterator end)
    {
        Container values;
        traits::detail::reserve_if_random_access_by_range(values, begin, end);

        std::transform(begin, end, std::back_inserter(values),
            traits::acquire_future_disp());

        return pika::when_some(n, PIKA_MOVE(values));
    }

    template <typename Iterator,
        typename Container = std::vector<
            typename lcos::detail::future_iterator_traits<Iterator>::type>,
        typename Enable =
            std::enable_if_t<pika::traits::is_iterator_v<Iterator>>>
    pika::future<when_some_result<Container>> when_some_n(
        std::size_t n, Iterator begin, std::size_t count)
    {
        Container values;
        traits::detail::reserve_if_reservable(values, count);

        traits::acquire_future_disp func;
        for (std::size_t i = 0; i != count; ++i)
        {
            values.push_back(func(*begin++));
        }

        return pika::when_some(n, PIKA_MOVE(values));
    }

    inline pika::future<when_some_result<pika::tuple<>>> when_some(std::size_t n)
    {
        using result_type = pika::tuple<>;

        if (n == 0)
        {
            return pika::make_ready_future(when_some_result<result_type>());
        }

        return pika::make_exceptional_future<when_some_result<result_type>>(
            PIKA_GET_EXCEPTION(pika::bad_parameter, "pika::when_some",
                "number of results to wait for is out of bounds"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename... Ts>
    typename std::enable_if<!(traits::is_future_range<T>::value &&
                                sizeof...(Ts) == 0),
        pika::future<when_some_result<
            pika::tuple<typename traits::acquire_future<T>::type,
                typename traits::acquire_future<Ts>::type...>>>>::type
    when_some(std::size_t n, T&& t, Ts&&... ts)
    {
        using result_type = pika::tuple<traits::acquire_future_t<T>,
            traits::acquire_future_t<Ts>...>;

        if (n == 0)
        {
            return pika::make_ready_future(when_some_result<result_type>());
        }

        if (n > 1 + sizeof...(Ts))
        {
            return pika::make_exceptional_future<when_some_result<result_type>>(
                PIKA_GET_EXCEPTION(pika::bad_parameter, "pika::when_some",
                    "number of results to wait for is out of bounds"));
        }

        traits::acquire_future_disp func;
        result_type values(
            func(PIKA_FORWARD(T, t)), func(PIKA_FORWARD(Ts, ts))...);

        auto f = std::make_shared<lcos::detail::when_some<result_type>>(
            PIKA_MOVE(values), n);

        lcos::local::futures_factory<when_some_result<result_type>()> p(
            [f = PIKA_MOVE(f)]() -> when_some_result<result_type> {
                return (*f)();
            });

        auto result = p.get_future();
        p.apply();

        return result;
    }
}    // namespace pika

namespace pika::lcos {

    template <typename Range>
    PIKA_DEPRECATED_V(
        0, 1, "pika::lcos::when_some is deprecated. Use pika::when_some instead.")
    std::enable_if_t<traits::is_future_range_v<Range>,
        pika::future<
            when_some_result<std::decay_t<Range>>>> when_some(std::size_t n,
        Range&& values, error_code& = throws)
    {
        return pika::when_some(n, PIKA_FORWARD(Range, values));
    }

    template <typename Iterator,
        typename Container = std::vector<
            typename lcos::detail::future_iterator_traits<Iterator>::type>,
        typename Enable =
            std::enable_if_t<pika::traits::is_iterator_v<Iterator>>>
    PIKA_DEPRECATED_V(
        0, 1, "pika::lcos::when_some is deprecated. Use pika::when_some instead.")
    pika::future<when_some_result<Container>> when_some(
        std::size_t n, Iterator begin, Iterator end, error_code& = throws)
    {
        return pika::when_some(n, begin, end);
    }

    template <typename Iterator,
        typename Container = std::vector<
            typename lcos::detail::future_iterator_traits<Iterator>::type>,
        typename Enable =
            std::enable_if_t<pika::traits::is_iterator_v<Iterator>>>
    PIKA_DEPRECATED_V(0, 1,
        "pika::lcos::when_some_n is deprecated. Use pika::when_some_n instead.")
    pika::future<when_some_result<Container>> when_some_n(
        std::size_t n, Iterator begin, std::size_t count, error_code& = throws)
    {
        return pika::when_some(n, begin, count);
    }

    PIKA_DEPRECATED_V(
        0, 1, "pika::lcos::when_some is deprecated. Use pika::when_some instead.")
    inline pika::future<when_some_result<pika::tuple<>>> when_some(
        std::size_t n, error_code& = throws)
    {
        return pika::when_some(n);
    }

    template <typename T, typename... Ts>
    PIKA_DEPRECATED_V(
        0, 1, "pika::lcos::when_some is deprecated. Use pika::when_some instead.")
    std::enable_if_t<!(traits::is_future_range_v<T> && sizeof...(Ts) == 0),
        pika::future<when_some_result<pika::tuple<traits::acquire_future_t<T>,
            traits::acquire_future_t<Ts>...>>>> when_some(std::size_t n, T&& t,
        Ts&&... ts)
    {
        return pika::when_some(n, PIKA_FORWARD(T, t), PIKA_FORWARD(Ts, ts)...);
    }

    template <typename T, typename... Ts>
    PIKA_DEPRECATED_V(
        0, 1, "pika::lcos::when_some is deprecated. Use pika::when_some instead.")
    std::enable_if_t<!(traits::is_future_range_v<T> && sizeof...(Ts) == 0),
        pika::future<when_some_result<pika::tuple<traits::acquire_future_t<T>,
            traits::acquire_future_t<Ts>...>>>> when_some(std::size_t n,
        error_code&, T&& t, Ts&&... ts)
    {
        return pika::when_some(n, PIKA_FORWARD(T, t), PIKA_FORWARD(Ts, ts)...);
    }

    template <typename Container>
    using when_some_result PIKA_DEPRECATED_V(0, 1,
        "pika::lcos::when_some_result is deprecated. Use pika::when_some_result "
        "instead.") = pika::when_some_result<Container>;
}    // namespace pika::lcos

#endif    // DOXYGEN
