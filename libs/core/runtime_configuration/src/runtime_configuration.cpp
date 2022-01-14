//  Copyright (c) 2005-2020 Hartmut Kaiser
//  Copyright (c)      2011 Bryce Adelstein-Lelbach
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/local/config/endian.hpp>
#include <hpx/assert.hpp>
#include <hpx/local/version.hpp>
#include <hpx/modules/filesystem.hpp>
#include <hpx/modules/itt_notify.hpp>
#include <hpx/prefix/find_prefix.hpp>
#include <hpx/preprocessor/expand.hpp>
#include <hpx/preprocessor/stringize.hpp>
#include <hpx/runtime_configuration/init_ini_data.hpp>
#include <hpx/runtime_configuration/runtime_configuration.hpp>
#include <hpx/runtime_configuration/runtime_mode.hpp>
#include <hpx/util/from_string.hpp>
#include <hpx/util/get_entry_as.hpp>

#include <boost/tokenizer.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#if defined(HPX_WINDOWS)
#include <process.h>
#elif defined(HPX_HAVE_UNISTD_H)
#include <unistd.h>
#endif

#include <limits>

///////////////////////////////////////////////////////////////////////////////
namespace hpx { namespace util {

    namespace detail {

        // CMake does not deal with explicit semicolons well, for this reason,
        // the paths are delimited with ':'. On Windows those need to be
        // converted to ';'.
        std::string convert_delimiters(std::string paths)
        {
#if defined(HPX_WINDOWS)
            std::replace(paths.begin(), paths.end(), ':', ';');
#endif
            return paths;
        }
    }    // namespace detail

    // pre-initialize entries with compile time based values
    void runtime_configuration::pre_initialize_ini()
    {
        if (!need_to_call_pre_initialize)
            return;

        std::vector<std::string> lines = {
            // clang-format off
            // create an empty application section
            "[application]",

            // create system and application instance specific entries
            "[system]",
            "pid = " + std::to_string(getpid()),
#if defined(__linux) || defined(linux) || defined(__linux__)
            "executable_prefix = " + get_executable_prefix(argv0),
#else
            "executable_prefix = " + get_executable_prefix(),
#endif
            // create default installation location and logging settings
            "[hpx]",
            "master_ini_path = $[system.executable_prefix]/",
            "master_ini_path_suffixes = /share/hpx" HPX_INI_PATH_DELIMITER
                "/../share/hpx",
#ifdef HPX_HAVE_ITTNOTIFY
            "use_itt_notify = ${HPX_HAVE_ITTNOTIFY:0}",
#endif
            "finalize_wait_time = ${HPX_FINALIZE_WAIT_TIME:-1.0}",
            "shutdown_timeout = ${HPX_SHUTDOWN_TIMEOUT:-1.0}",
            "shutdown_check_count = ${HPX_SHUTDOWN_CHECK_COUNT:10}",
#ifdef HPX_HAVE_VERIFY_LOCKS
#if defined(HPX_DEBUG)
            "lock_detection = ${HPX_LOCK_DETECTION:1}",
#else
            "lock_detection = ${HPX_LOCK_DETECTION:0}",
#endif
            "throw_on_held_lock = ${HPX_THROW_ON_HELD_LOCK:1}",
#endif
#ifdef HPX_HAVE_THREAD_MINIMAL_DEADLOCK_DETECTION
#ifdef HPX_DEBUG
            "minimal_deadlock_detection = ${HPX_MINIMAL_DEADLOCK_DETECTION:1}",
#else
            "minimal_deadlock_detection = ${HPX_MINIMAL_DEADLOCK_DETECTION:0}",
#endif
#endif
#ifdef HPX_HAVE_SPINLOCK_DEADLOCK_DETECTION
#ifdef HPX_DEBUG
            "spinlock_deadlock_detection = "
            "${HPX_SPINLOCK_DEADLOCK_DETECTION:1}",
#else
            "spinlock_deadlock_detection = "
            "${HPX_SPINLOCK_DEADLOCK_DETECTION:0}",
#endif
            "spinlock_deadlock_detection_limit = "
            "${HPX_SPINLOCK_DEADLOCK_DETECTION_LIMIT:" HPX_PP_STRINGIZE(
                HPX_PP_EXPAND(HPX_SPINLOCK_DEADLOCK_DETECTION_LIMIT)) "}",
#endif
            "expect_connecting_localities = "
            "${HPX_EXPECT_CONNECTING_LOCALITIES:0}",

            // add placeholders for keys to be added by command line handling
            "os_threads = cores",
            "cores = all",
            "localities = 1",
            "first_pu = 0",
            "runtime_mode = console",
            "scheduler = local-priority-fifo",
            "affinity = core",
            "pu_step = 1",
            "pu_offset = 0",
            "numa_sensitive = 0",
            "max_background_threads = "
            "${HPX_MAX_BACKGROUND_THREADS:$[hpx.os_threads]}",
            "max_idle_loop_count = ${HPX_MAX_IDLE_LOOP_COUNT:" HPX_PP_STRINGIZE(
                HPX_PP_EXPAND(HPX_IDLE_LOOP_COUNT_MAX)) "}",
            "max_busy_loop_count = ${HPX_MAX_BUSY_LOOP_COUNT:" HPX_PP_STRINGIZE(
                HPX_PP_EXPAND(HPX_BUSY_LOOP_COUNT_MAX)) "}",
#if defined(HPX_HAVE_THREAD_MANAGER_IDLE_BACKOFF)
            "max_idle_backoff_time = "
            "${HPX_MAX_IDLE_BACKOFF_TIME:" HPX_PP_STRINGIZE(
                HPX_PP_EXPAND(HPX_IDLE_BACKOFF_TIME_MAX)) "}",
#endif
            "default_scheduler_mode = ${HPX_DEFAULT_SCHEDULER_MODE}",

        /// If HPX_HAVE_ATTACH_DEBUGGER_ON_TEST_FAILURE is set,
        /// then apply the test-failure value as default.
#if defined(HPX_HAVE_ATTACH_DEBUGGER_ON_TEST_FAILURE)
            "attach_debugger = ${HPX_ATTACH_DEBUGGER:test-failure}",
#else
            "attach_debugger = ${HPX_ATTACH_DEBUGGER}",
#endif
            "exception_verbosity = ${HPX_EXCEPTION_VERBOSITY:2}",
            "trace_depth = ${HPX_TRACE_DEPTH:" HPX_PP_STRINGIZE(
                HPX_PP_EXPAND(HPX_HAVE_THREAD_BACKTRACE_DEPTH)) "}",

            "[hpx.stacks]",
            "small_size = ${HPX_SMALL_STACK_SIZE:" HPX_PP_STRINGIZE(
                HPX_PP_EXPAND(HPX_SMALL_STACK_SIZE)) "}",
            "medium_size = ${HPX_MEDIUM_STACK_SIZE:" HPX_PP_STRINGIZE(
                HPX_PP_EXPAND(HPX_MEDIUM_STACK_SIZE)) "}",
            "large_size = ${HPX_LARGE_STACK_SIZE:" HPX_PP_STRINGIZE(
                HPX_PP_EXPAND(HPX_LARGE_STACK_SIZE)) "}",
            "huge_size = ${HPX_HUGE_STACK_SIZE:" HPX_PP_STRINGIZE(
                HPX_PP_EXPAND(HPX_HUGE_STACK_SIZE)) "}",
#if defined(__linux) || defined(linux) || defined(__linux__) ||                \
    defined(__FreeBSD__)
            "use_guard_pages = ${HPX_USE_GUARD_PAGES:1}",
#endif

            "[hpx.thread_queue]",
            "max_thread_count = ${HPX_THREAD_QUEUE_MAX_THREAD_COUNT:" HPX_PP_STRINGIZE(
                HPX_PP_EXPAND(HPX_THREAD_QUEUE_MAX_THREAD_COUNT)) "}",
            "min_tasks_to_steal_pending = "
            "${HPX_THREAD_QUEUE_MIN_TASKS_TO_STEAL_PENDING:" HPX_PP_STRINGIZE(
                HPX_PP_EXPAND(HPX_THREAD_QUEUE_MIN_TASKS_TO_STEAL_PENDING)) "}",
            "min_tasks_to_steal_staged = "
            "${HPX_THREAD_QUEUE_MIN_TASKS_TO_STEAL_STAGED:" HPX_PP_STRINGIZE(
                HPX_PP_EXPAND(HPX_THREAD_QUEUE_MIN_TASKS_TO_STEAL_STAGED)) "}",
            "min_add_new_count = "
            "${HPX_THREAD_QUEUE_MIN_ADD_NEW_COUNT:" HPX_PP_STRINGIZE(
                HPX_PP_EXPAND(HPX_THREAD_QUEUE_MIN_ADD_NEW_COUNT)) "}",
            "max_add_new_count = "
            "${HPX_THREAD_QUEUE_MAX_ADD_NEW_COUNT:" HPX_PP_STRINGIZE(
                HPX_PP_EXPAND(HPX_THREAD_QUEUE_MAX_ADD_NEW_COUNT)) "}",
            "min_delete_count = "
            "${HPX_THREAD_QUEUE_MIN_DELETE_COUNT:" HPX_PP_STRINGIZE(
                HPX_PP_EXPAND(HPX_THREAD_QUEUE_MIN_DELETE_COUNT)) "}",
            "max_delete_count = "
            "${HPX_THREAD_QUEUE_MAX_DELETE_COUNT:" HPX_PP_STRINGIZE(
                HPX_PP_EXPAND(HPX_THREAD_QUEUE_MAX_THREAD_COUNT)) "}",
            "max_terminated_threads = "
            "${HPX_THREAD_QUEUE_MAX_TERMINATED_THREADS:" HPX_PP_STRINGIZE(
                HPX_PP_EXPAND(HPX_THREAD_QUEUE_MAX_TERMINATED_THREADS)) "}",
            "init_threads_count = "
            "${HPX_THREAD_QUEUE_INIT_THREADS_COUNT:" HPX_PP_STRINGIZE(
                HPX_PP_EXPAND(HPX_THREAD_QUEUE_INIT_THREADS_COUNT)) "}",

            "[hpx.commandline]",
            // enable aliasing
            "aliasing = ${HPX_COMMANDLINE_ALIASING:1}",

            // allow for unknown options to be passed through
            "allow_unknown = ${HPX_COMMANDLINE_ALLOW_UNKNOWN:0}",

            // allow for command line options to to be passed through the
            // environment
            "prepend_options = ${HPX_COMMANDLINE_OPTIONS}",

            // predefine command line aliases
            "[hpx.commandline.aliases]",
            "-h = --hpx:help",
            "-I = --hpx:ini",
            "-p = --hpx:app-config",
            "-q = --hpx:queuing",
            "-t = --hpx:threads",
            "-v = --hpx:version",
            "-x = --hpx:hpx",
            "-0 = --hpx:node=0",
            "-1 = --hpx:node=1",
            "-2 = --hpx:node=2",
            "-3 = --hpx:node=3",
            "-4 = --hpx:node=4",
            "-5 = --hpx:node=5",
            "-6 = --hpx:node=6",
            "-7 = --hpx:node=7",
            "-8 = --hpx:node=8",
            "-9 = --hpx:node=9",
            // clang-format on
        };

        lines.insert(lines.end(), extra_static_ini_defs.begin(),
            extra_static_ini_defs.end());

        // don't overload user overrides
        this->parse("<static defaults>", lines, false, false, false);

        need_to_call_pre_initialize = false;
    }

    void runtime_configuration::post_initialize_ini(std::string& hpx_ini_file_,
        std::vector<std::string> const& cmdline_ini_defs_)
    {
        util::init_ini_data_base(*this, hpx_ini_file_);
        need_to_call_pre_initialize = true;

        // let the command line override the config file.
        if (!cmdline_ini_defs_.empty())
        {
            // do not weed out comments
            this->parse(
                "<command line definitions>", cmdline_ini_defs_, true, false);
            need_to_call_pre_initialize = true;
        }
    }

    void runtime_configuration::pre_initialize_logging_ini()
    {
#if defined(HPX_LOCAL_HAVE_LOGGING)
        std::vector<std::string> lines = {
        // clang-format off
#define HPX_TIMEFORMAT "$hh:$mm.$ss.$mili"
#define HPX_LOGFORMAT "(T%locality%/%hpxthread%.%hpxphase%) "

            // general logging
            "[hpx.logging]",
            "level = ${HPX_LOGLEVEL:0}",
            "destination = ${HPX_LOGDESTINATION:console}",
            "format = ${HPX_LOGFORMAT:" HPX_LOGFORMAT
                "P%parentloc%/%hpxparent%.%hpxparentphase% %time%("
                HPX_TIMEFORMAT ") [%idx%]|\\n}",

            // general console logging
            "[hpx.logging.console]",
            "level = ${HPX_LOGLEVEL:$[hpx.logging.level]}",
#if defined(ANDROID) || defined(__ANDROID__)
            "destination = ${HPX_CONSOLE_LOGDESTINATION:android_log}",
#else
            "destination = ${HPX_CONSOLE_LOGDESTINATION:"
                "file(hpx.$[system.pid].log)}",
#endif
            "format = ${HPX_CONSOLE_LOGFORMAT:|}",

            // logging related to timing
            "[hpx.logging.timing]",
            "level = ${HPX_TIMING_LOGLEVEL:-1}",
            "destination = ${HPX_TIMING_LOGDESTINATION:console}",
            "format = ${HPX_TIMING_LOGFORMAT:" HPX_LOGFORMAT
                "P%parentloc%/%hpxparent%.%hpxparentphase% %time%("
                HPX_TIMEFORMAT ") [%idx%] [TIM] |\\n}",

            // console logging related to timing
            "[hpx.logging.console.timing]",
            "level = ${HPX_TIMING_LOGLEVEL:$[hpx.logging.timing.level]}",
#if defined(ANDROID) || defined(__ANDROID__)
            "destination = ${HPX_CONSOLE_TIMING_LOGDESTINATION:android_log}",
#else
            "destination = ${HPX_CONSOLE_TIMING_LOGDESTINATION:"
                "file(hpx.timing.$[system.pid].log)}",
#endif
            "format = ${HPX_CONSOLE_TIMING_LOGFORMAT:|}",

            // logging related to applications
            "[hpx.logging.application]",
            "level = ${HPX_APP_LOGLEVEL:-1}",
            "destination = ${HPX_APP_LOGDESTINATION:console}",
            "format = ${HPX_APP_LOGFORMAT:" HPX_LOGFORMAT
                "P%parentloc%/%hpxparent%.%hpxparentphase% %time%("
                HPX_TIMEFORMAT ") [%idx%] [APP] |\\n}",

            // console logging related to applications
            "[hpx.logging.console.application]",
            "level = ${HPX_APP_LOGLEVEL:$[hpx.logging.application.level]}",
#if defined(ANDROID) || defined(__ANDROID__)
            "destination = ${HPX_CONSOLE_APP_LOGDESTINATION:android_log}",
#else
            "destination = ${HPX_CONSOLE_APP_LOGDESTINATION:"
                "file(hpx.application.$[system.pid].log)}",
#endif
            "format = ${HPX_CONSOLE_APP_LOGFORMAT:|}",

            // logging of debug channel
            "[hpx.logging.debuglog]",
            "level = ${HPX_DEB_LOGLEVEL:-1}",
            "destination = ${HPX_DEB_LOGDESTINATION:console}",
            "format = ${HPX_DEB_LOGFORMAT:" HPX_LOGFORMAT
                "P%parentloc%/%hpxparent%.%hpxparentphase% %time%("
                HPX_TIMEFORMAT ") [%idx%] [DEB] |\\n}",

            "[hpx.logging.console.debuglog]",
            "level = ${HPX_DEB_LOGLEVEL:$[hpx.logging.debuglog.level]}",
#if defined(ANDROID) || defined(__ANDROID__)
            "destination = ${HPX_CONSOLE_DEB_LOGDESTINATION:android_log}",
#else
            "destination = ${HPX_CONSOLE_DEB_LOGDESTINATION:"
                "file(hpx.debuglog.$[system.pid].log)}",
#endif
            "format = ${HPX_CONSOLE_DEB_LOGFORMAT:|}"

#undef HPX_TIMEFORMAT
#undef HPX_LOGFORMAT
            // clang-format on
        };

        // don't overload user overrides
        this->parse("<static logging defaults>", lines, false, false);
#endif
    }

    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    runtime_configuration::runtime_configuration(char const* argv0_,
        runtime_mode mode,
        std::vector<std::string> const& extra_static_ini_defs_)
      : extra_static_ini_defs(extra_static_ini_defs_)
      , mode_(mode)
      , num_os_threads(0)
      , small_stacksize(HPX_SMALL_STACK_SIZE)
      , medium_stacksize(HPX_MEDIUM_STACK_SIZE)
      , large_stacksize(HPX_LARGE_STACK_SIZE)
      , huge_stacksize(HPX_HUGE_STACK_SIZE)
      , need_to_call_pre_initialize(true)
#if defined(__linux) || defined(linux) || defined(__linux__)
      , argv0(argv0_)
#endif
    {
        pre_initialize_ini();

        // set global config options
#if HPX_HAVE_ITTNOTIFY != 0
        use_ittnotify_api = get_itt_notify_mode();
#endif
        HPX_ASSERT(init_small_stack_size() >= HPX_SMALL_STACK_SIZE);

        small_stacksize = init_small_stack_size();
        medium_stacksize = init_medium_stack_size();
        large_stacksize = init_large_stack_size();
        HPX_ASSERT(init_huge_stack_size() <= HPX_HUGE_STACK_SIZE);
        huge_stacksize = init_huge_stack_size();
    }

    ///////////////////////////////////////////////////////////////////////////
    void runtime_configuration::reconfigure(std::string const& hpx_ini_file_)
    {
        hpx_ini_file = hpx_ini_file_;
        reconfigure();
    }

    void runtime_configuration::reconfigure(
        std::vector<std::string> const& cmdline_ini_defs_)
    {
        cmdline_ini_defs = cmdline_ini_defs_;
        reconfigure();
    }

    void runtime_configuration::reconfigure()
    {
        pre_initialize_ini();
        pre_initialize_logging_ini();
        post_initialize_ini(hpx_ini_file, cmdline_ini_defs);

        // set global config options
#if HPX_HAVE_ITTNOTIFY != 0
        use_ittnotify_api = get_itt_notify_mode();
#endif
        HPX_ASSERT(init_small_stack_size() >= HPX_SMALL_STACK_SIZE);

        small_stacksize = init_small_stack_size();
        medium_stacksize = init_medium_stack_size();
        large_stacksize = init_large_stack_size();
        huge_stacksize = init_huge_stack_size();
    }

    bool runtime_configuration::get_itt_notify_mode() const
    {
#if HPX_HAVE_ITTNOTIFY != 0
        if (util::section const* sec = get_section("hpx"); nullptr != sec)
        {
            return hpx::util::get_entry_as<int>(*sec, "use_itt_notify", 0) != 0;
        }
#endif
        return false;
    }

    // Enable lock detection during suspension
    bool runtime_configuration::enable_lock_detection() const
    {
#ifdef HPX_HAVE_VERIFY_LOCKS
        if (util::section const* sec = get_section("hpx"); nullptr != sec)
        {
            return hpx::util::get_entry_as<int>(*sec, "lock_detection", 0) != 0;
        }
#endif
        return false;
    }

    // Enable minimal deadlock detection for HPX threads
    bool runtime_configuration::enable_minimal_deadlock_detection() const
    {
#ifdef HPX_HAVE_THREAD_MINIMAL_DEADLOCK_DETECTION
        if (util::section const* sec = get_section("hpx"); nullptr != sec)
        {
#ifdef HPX_DEBUG
            return hpx::util::get_entry_as<int>(
                       *sec, "minimal_deadlock_detection", 1) != 0;
#else
            return hpx::util::get_entry_as<int>(
                       *sec, "minimal_deadlock_detection", 0) != 0;
#endif
        }

#ifdef HPX_DEBUG
        return true;
#else
        return false;
#endif

#else
        return false;
#endif
    }

    ///////////////////////////////////////////////////////////////////////////
    bool runtime_configuration::enable_spinlock_deadlock_detection() const
    {
#ifdef HPX_HAVE_SPINLOCK_DEADLOCK_DETECTION
        if (util::section const* sec = get_section("hpx"); nullptr != sec)
        {
#ifdef HPX_DEBUG
            return hpx::util::get_entry_as<int>(
                       *sec, "spinlock_deadlock_detection", 1) != 0;
#else
            return hpx::util::get_entry_as<int>(
                       *sec, "spinlock_deadlock_detection", 0) != 0;
#endif
        }

#ifdef HPX_DEBUG
        return true;
#else
        return false;
#endif

#else
        return false;
#endif
    }

    ///////////////////////////////////////////////////////////////////////////
    std::size_t runtime_configuration::get_spinlock_deadlock_detection_limit()
        const
    {
#ifdef HPX_HAVE_SPINLOCK_DEADLOCK_DETECTION
        if (util::section const* sec = get_section("hpx"); nullptr != sec)
        {
            return hpx::util::get_entry_as<std::size_t>(*sec,
                "spinlock_deadlock_detection_limit",
                HPX_SPINLOCK_DEADLOCK_DETECTION_LIMIT);
        }
        return HPX_SPINLOCK_DEADLOCK_DETECTION_LIMIT;
#else
        return std::size_t(-1);
#endif
    }

    std::size_t runtime_configuration::trace_depth() const
    {
        if (util::section const* sec = get_section("hpx"); nullptr != sec)
        {
            return hpx::util::get_entry_as<std::size_t>(
                *sec, "trace_depth", HPX_HAVE_THREAD_BACKTRACE_DEPTH);
        }
        return HPX_HAVE_THREAD_BACKTRACE_DEPTH;
    }

    std::size_t runtime_configuration::get_os_thread_count() const
    {
        if (num_os_threads == 0)
        {
            num_os_threads = 1;
            if (util::section const* sec = get_section("hpx"); nullptr != sec)
            {
                num_os_threads = hpx::util::get_entry_as<std::uint32_t>(
                    *sec, "os_threads", 1);
            }
        }
        return static_cast<std::size_t>(num_os_threads);
    }

    std::string runtime_configuration::get_cmd_line() const
    {
        if (util::section const* sec = get_section("hpx"); nullptr != sec)
        {
            return sec->get_entry("cmd_line", "");
        }
        return "";
    }

    // Return the configured sizes of any of the know thread pools
    std::size_t runtime_configuration::get_thread_pool_size(
        char const* poolname) const
    {
        if (util::section const* sec = get_section("hpx.threadpools");
            nullptr != sec)
        {
            return hpx::util::get_entry_as<std::size_t>(
                *sec, std::string(poolname) + "_size", 2);
        }
        return 2;    // the default size for all pools is 2
    }

    // Will return the stack size to use for all HPX-threads.
    std::ptrdiff_t runtime_configuration::init_stack_size(char const* entryname,
        char const* defaultvaluestr, std::ptrdiff_t defaultvalue) const
    {
        if (util::section const* sec = get_section("hpx.stacks");
            nullptr != sec)
        {
            std::string entry = sec->get_entry(entryname, defaultvaluestr);
            char* endptr = nullptr;
            std::ptrdiff_t val =
                std::strtoll(entry.c_str(), &endptr, /*base:*/ 0);
            return endptr != entry.c_str() ? val : defaultvalue;
        }
        return defaultvalue;
    }

#if defined(__linux) || defined(linux) || defined(__linux__) ||                \
    defined(__FreeBSD__)
    bool runtime_configuration::use_stack_guard_pages() const
    {
        if (util::section const* sec = get_section("hpx.stacks");
            nullptr != sec)
        {
            return hpx::util::get_entry_as<int>(*sec, "use_guard_pages", 1) !=
                0;
        }
        return true;    // default is true
    }
#endif

    std::ptrdiff_t runtime_configuration::init_small_stack_size() const
    {
        return init_stack_size("small_size",
            HPX_PP_STRINGIZE(HPX_SMALL_STACK_SIZE), HPX_SMALL_STACK_SIZE);
    }

    std::ptrdiff_t runtime_configuration::init_medium_stack_size() const
    {
        return init_stack_size("medium_size",
            HPX_PP_STRINGIZE(HPX_MEDIUM_STACK_SIZE), HPX_MEDIUM_STACK_SIZE);
    }

    std::ptrdiff_t runtime_configuration::init_large_stack_size() const
    {
        return init_stack_size("large_size",
            HPX_PP_STRINGIZE(HPX_LARGE_STACK_SIZE), HPX_LARGE_STACK_SIZE);
    }

    std::ptrdiff_t runtime_configuration::init_huge_stack_size() const
    {
        return init_stack_size("huge_size",
            HPX_PP_STRINGIZE(HPX_HUGE_STACK_SIZE), HPX_HUGE_STACK_SIZE);
    }

    ///////////////////////////////////////////////////////////////////////////
    bool runtime_configuration::load_application_configuration(
        char const* filename, error_code& ec)
    {
        try
        {
            section appcfg(filename);
            section applroot;
            applroot.add_section("application", appcfg);
            this->section::merge(applroot);
        }
        catch (hpx::exception const& e)
        {
            // file doesn't exist or is ill-formed
            if (&ec == &throws)
                throw;
            ec = make_error_code(e.get_error(), e.what(), hpx::rethrow);
            return false;
        }
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////
    std::ptrdiff_t runtime_configuration::get_stack_size(
        threads::thread_stacksize stacksize) const
    {
        switch (stacksize)
        {
        case threads::thread_stacksize::medium:
            return medium_stacksize;

        case threads::thread_stacksize::large:
            return large_stacksize;

        case threads::thread_stacksize::huge:
            return huge_stacksize;

        case threads::thread_stacksize::nostack:
            return (std::numeric_limits<std::ptrdiff_t>::max)();

        default:
        case threads::thread_stacksize::small_:
            break;
        }
        return small_stacksize;
    }
}}    // namespace hpx::util
