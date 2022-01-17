////////////////////////////////////////////////////////////////////////////////
//  Copyright (c) 2011 Bryce Lelbach
//  Copyright (c) 2011-2017 Hartmut Kaiser
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
////////////////////////////////////////////////////////////////////////////////

#include <pika/local/config.hpp>
#include <pika/local/config/config_strings.hpp>
#include <pika/local/config/version.hpp>
#include <pika/local/version.hpp>
#include <pika/modules/format.hpp>
#include <pika/prefix/find_prefix.hpp>
#include <pika/preprocessor/stringize.hpp>

#include <boost/config.hpp>
#include <boost/version.hpp>

#if defined(PIKA_HAVE_MODULE_MPI_BASE)
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-qual"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif

// Intel MPI does not like to be included after stdio.h. As such, we include mpi.h
// as soon as possible.
#include <mpi.h>

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif

#include <hwloc.h>

#include <algorithm>
#include <cstdint>
#include <sstream>
#include <string>

///////////////////////////////////////////////////////////////////////////////
namespace pika::local {
    std::uint8_t major_version()
    {
        return PIKA_VERSION_MAJOR;
    }

    std::uint8_t minor_version()
    {
        return PIKA_VERSION_MINOR;
    }

    std::uint8_t subminor_version()
    {
        return PIKA_VERSION_SUBMINOR;
    }

    std::uint32_t full_version()
    {
        return PIKA_VERSION_FULL;
    }

    std::string full_version_as_string()
    {
        return pika::util::format("{}.{}.{}",    //-V609
            PIKA_VERSION_MAJOR, PIKA_VERSION_MINOR,
            PIKA_VERSION_SUBMINOR);
    }

    std::string tag()
    {
        return PIKA_VERSION_TAG;
    }

#if defined(PIKA_HAVE_MODULE_MPI_BASE)
    std::string mpi_version()
    {
        std::ostringstream strm;

        // add type and library version
#if defined(OPEN_MPI)
        pika::util::format_to(strm, "OpenMPI V{}.{}.{}", OMPI_MAJOR_VERSION,
            OMPI_MINOR_VERSION, OMPI_RELEASE_VERSION);
#elif defined(MPICH)
        pika::util::format_to(strm, "MPICH V{}", MPICH_VERSION);
#elif defined(MVAPICH2_VERSION)
        pika::util::format_to(strm, "MVAPICH2 V{}", MVAPICH2_VERSION);
#else
        strm << "Unknown MPI";
#endif
        // add general MPI version
#if defined(MPI_VERSION) && defined(MPI_SUBVERSION)
        pika::util::format_to(strm, ", MPI V{}.{}", MPI_VERSION, MPI_SUBVERSION);
#else
        strm << ", unknown MPI version";
#endif
        return strm.str();
    }
#endif

    std::string copyright()
    {
        char const* const copyright =
            "pika\n\n"
            "Copyright (c) 2021-2022, ETH Zurich,\n"
            "https://github.com/pika-org/pika\n\n"
            "Distributed under the Boost Software License, "
            "Version 1.0. (See accompanying\n"
            "file LICENSE_1_0.txt or copy at "
            "http://www.boost.org/LICENSE_1_0.txt)\n";
        return copyright;
    }

    // Returns the pika full build information string.
    std::string full_build_string()
    {
        std::ostringstream strm;
        strm << "{config}:\n"
             << configuration_string() << "{version}: " << build_string()
             << "\n"
             << "{boost}: " << boost_version() << "\n"
             << "{build-type}: " << build_type() << "\n"
             << "{date}: " << build_date_time() << "\n"
             << "{platform}: " << boost_platform() << "\n"
             << "{compiler}: " << boost_compiler() << "\n"
             << "{stdlib}: " << boost_stdlib() << "\n";

        return strm.str();
    }

    ///////////////////////////////////////////////////////////////////////////
    std::string configuration_string()
    {
        std::ostringstream strm;

        strm << "pika:\n";

#if defined(PIKA_HAVE_MALLOC)
        pika::util::format_to(strm, "  PIKA_HAVE_MALLOC={}\n", PIKA_HAVE_MALLOC);
#endif

        char const* const* p = pika::local::config_strings;
        while (*p)
            strm << "  " << *p++ << "\n";
        strm << "\n";

        return strm.str();
    }

    std::string build_string()
    {
        return pika::util::format("V{}{}, Git: {:.10}",    //-V609
            full_version_as_string(), PIKA_VERSION_TAG,
            PIKA_HAVE_GIT_COMMIT);
    }

    std::string boost_version()
    {
        // BOOST_VERSION: 107100
        return pika::util::format("V{}.{}.{}", BOOST_VERSION / 100000,
            BOOST_VERSION / 100 % 1000, BOOST_VERSION % 100);
    }

    std::string hwloc_version()
    {
        // HWLOC_API_VERSION: 0x00010700
        return pika::util::format("V{}.{}.{}", HWLOC_API_VERSION / 0x10000,
            HWLOC_API_VERSION / 0x100 % 0x100, HWLOC_API_VERSION % 0x100);
    }

#if defined(PIKA_HAVE_MALLOC)
    std::string malloc_version()
    {
        return PIKA_HAVE_MALLOC;
    }
#endif

    std::string boost_platform()
    {
        return BOOST_PLATFORM;
    }

    std::string boost_compiler()
    {
        return BOOST_COMPILER;
    }

    std::string boost_stdlib()
    {
        return BOOST_STDLIB;
    }

    std::string complete_version()
    {
        std::string version = pika::util::format("Versions:\n"
                                                "  pika: {}\n"
                                                "  Boost: {}\n"
                                                "  Hwloc: {}\n"
#if defined(PIKA_HAVE_MODULE_MPI_BASE)
                                                "  MPI: {}\n"
#endif
                                                "\n"
                                                "Build:\n"
                                                "  Type: {}\n"
                                                "  Date: {}\n"
                                                "  Platform: {}\n"
                                                "  Compiler: {}\n"
                                                "  Standard Library: {}\n",
            build_string(), boost_version(), hwloc_version(),
#if defined(PIKA_HAVE_MODULE_MPI_BASE)
            mpi_version(),
#endif
            build_type(), build_date_time(), boost_platform(), boost_compiler(),
            boost_stdlib());

#if defined(PIKA_HAVE_MALLOC)
        version += "  Allocator: " + malloc_version() + "\n";
#endif

        return version;
    }

    std::string build_type()
    {
        return PIKA_PP_STRINGIZE(PIKA_BUILD_TYPE);
    }

    std::string build_date_time()
    {
        return std::string(__DATE__) + " " + __TIME__;
    }
}    // namespace pika::local
