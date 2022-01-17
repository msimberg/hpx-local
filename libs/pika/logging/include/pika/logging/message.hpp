// message.hpp

// Boost Logging library
//
// Author: John Torjo, www.torjo.com
//
// Copyright (C) 2007 John Torjo (see www.torjo.com for email)
//
//  SPDX-License-Identifier: BSL-1.0
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org for updates, documentation, and revision history.
// See http://www.torjo.com/log2/ for more details

#pragma once

#include <pika/local/config.hpp>
#include <pika/modules/format.hpp>
#include <pika/type_support/unused.hpp>

#include <cstddef>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

namespace pika { namespace util { namespace logging {

    /**
        @brief Optimizes the formatting for prepending and/or appending strings to
        the original message

        It keeps all the modified message in one string.
        Useful if some formatter needs to access the whole
        string at once.

        reserve() - the size that is reserved for prepending
        (similar to string::reserve function)

        Note : as strings are prepended, reserve() shrinks.
    */
    class message
    {
    public:
        message()
          : m_full_msg_computed(false)
        {
        }

        /**
        @param msg - the message that is originally cached
         */
        explicit message(std::stringstream msg)
          :
#if defined(PIKA_COMPUTE_HOST_CODE)
          m_str(PIKA_MOVE(msg))
          ,
#endif
          m_full_msg_computed(false)
        {
#if !defined(PIKA_COMPUTE_HOST_CODE)
            PIKA_UNUSED(msg);
#endif
        }

        message(message&& other) noexcept
          :
#if defined(PIKA_COMPUTE_HOST_CODE)
          m_str(PIKA_MOVE(other.m_str))
          ,
#endif
          m_full_msg_computed(other.m_full_msg_computed)
#if defined(PIKA_COMPUTE_HOST_CODE)
          , m_full_msg(PIKA_MOVE(other.m_full_msg))
#endif
        {
            other.m_full_msg_computed = false;
        }

        template <typename T>
        message& operator<<(T&& v)
        {
            m_str << PIKA_FORWARD(T, v);
            m_full_msg_computed = false;
            return *this;
        }

        template <typename... Args>
        message& format(
            std::string_view format_str, Args const&... args) noexcept
        {
            util::format_to(m_str, format_str, args...);
            m_full_msg_computed = false;
            return *this;
        }

        /**
            returns the full string
        */
        std::string const& full_string() const
        {
            if (!m_full_msg_computed)
            {
                m_full_msg_computed = true;
                m_full_msg = m_str.str();
            }
            return m_full_msg;
        }

        bool empty() const
        {
            return full_string().empty();
        }

        friend std::ostream& operator<<(std::ostream& os, message const& value)
        {
            return os << value.m_str.rdbuf();
        }

    private:
        std::stringstream m_str;

        // caching
        mutable bool m_full_msg_computed;
        mutable std::string m_full_msg;
    };
}}}    // namespace pika::util::logging
