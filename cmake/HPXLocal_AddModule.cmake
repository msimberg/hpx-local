# Copyright (c) 2019 ETH Zurich
#
# SPDX-License-Identifier: BSL-1.0
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

include(HPXLocal_ExportTargets)

function(hpx_local_add_module libname modulename)
  # Retrieve arguments
  set(options CUDA CONFIG_FILES)
  set(one_value_args GLOBAL_HEADER_GEN)
  set(multi_value_args
      SOURCES
      HEADERS
      COMPAT_HEADERS
      OBJECTS
      DEPENDENCIES
      MODULE_DEPENDENCIES
      CMAKE_SUBDIRS
      EXCLUDE_FROM_GLOBAL_HEADER
  )
  cmake_parse_arguments(
    ${modulename} "${options}" "${one_value_args}" "${multi_value_args}"
    ${ARGN}
  )
  if(${modulename}_UNPARSED_ARGUMENTS)
    message(
      AUTHOR_WARNING
        "Arguments were not used by the module: ${${modulename}_UNPARSED_ARGUMENTS}"
    )
  endif()

  include(HPXLocal_Message)
  include(HPXLocal_Option)

  # Global headers should be always generated except if explicitly disabled
  if("${${modulename}_GLOBAL_HEADER_GEN}" STREQUAL "")
    set(${modulename}_GLOBAL_HEADER_GEN ON)
  endif()

  string(TOUPPER ${libname} libname_upper)
  string(TOUPPER ${modulename} modulename_upper)

  # Mark the module as enabled (see hpx/libs/CMakeLists.txt)
  set(HPXLocal_ENABLED_MODULES
      ${HPXLocal_ENABLED_MODULES} ${modulename}
      CACHE INTERNAL "List of enabled HPXLocal modules" FORCE
  )

  # Main directories of the module
  set(SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/src")
  set(HEADER_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/include")

  hpx_local_debug("Add module ${modulename}: SOURCE_ROOT: ${SOURCE_ROOT}")
  hpx_local_debug("Add module ${modulename}: HEADER_ROOT: ${HEADER_ROOT}")

  if(${modulename}_COMPAT_HEADERS)
    set(COMPAT_HEADER_ROOT "${CMAKE_CURRENT_BINARY_DIR}/include_compatibility")
    file(MAKE_DIRECTORY ${COMPAT_HEADER_ROOT})
    hpx_local_debug(
      "Add module ${modulename}: COMPAT_HEADER_ROOT: ${COMPAT_HEADER_ROOT}"
    )
  endif()

  set(all_headers ${${modulename}_HEADERS})

  # Write full path for the sources files
  list(TRANSFORM ${modulename}_SOURCES PREPEND ${SOURCE_ROOT}/ OUTPUT_VARIABLE
                                                               sources
  )
  list(TRANSFORM ${modulename}_HEADERS PREPEND ${HEADER_ROOT}/ OUTPUT_VARIABLE
                                                               headers
  )
  if(${modulename}_COMPAT_HEADERS)
    string(REPLACE ";=>;" "=>" ${modulename}_COMPAT_HEADERS
                   "${${modulename}_COMPAT_HEADERS}"
    )
    foreach(compat_header IN LISTS ${modulename}_COMPAT_HEADERS)
      string(REPLACE "=>" ";" compat_header "${compat_header}")
      list(LENGTH compat_header compat_header_length)
      if(NOT compat_header_length EQUAL 2)
        message(FATAL_ERROR "Invalid compatibility header ${compat_header}")
      endif()

      list(GET compat_header 0 old_header)
      list(GET compat_header 1 new_header)
      configure_file(
        "${PROJECT_SOURCE_DIR}/cmake/templates/compatibility_header.hpp.in"
        "${COMPAT_HEADER_ROOT}/${old_header}"
      )
      list(APPEND compat_headers "${COMPAT_HEADER_ROOT}/${old_header}")
      list(APPEND all_headers ${old_header})
    endforeach()
  endif()

  # This header generation is disabled for config module specific generated
  # headers are included
  if(${modulename}_GLOBAL_HEADER_GEN)
    if("hpx/modules/${modulename}.hpp" IN_LIST all_headers)
      string(
        CONCAT
          error_message
          "Global header generation turned on for module ${modulename} but the "
          "header \"hpx/modules/${modulename}.hpp\" is also listed explicitly as"
          "a header. Turn off global header generation or remove the "
          "\"hpx/modules/${modulename}.hpp\" file."
      )
      hpx_local_error(${error_message})
    endif()
    # Add a global include file that include all module headers
    set(global_header
        "${CMAKE_CURRENT_BINARY_DIR}/include/hpx/modules/${modulename}.hpp"
    )
    set(module_headers)
    foreach(header_file ${${modulename}_HEADERS})
      # Exclude the files specified
      if((NOT (${header_file} IN_LIST ${modulename}_EXCLUDE_FROM_GLOBAL_HEADER))
         AND (NOT ("${header_file}" MATCHES "detail"))
      )
        set(module_headers "${module_headers}#include <${header_file}>\n")
      endif()
    endforeach(header_file)
    configure_file(
      "${PROJECT_SOURCE_DIR}/cmake/templates/global_module_header.hpp.in"
      "${global_header}"
    )
    set(generated_headers ${global_header})
  endif()

  # generate configuration header for this module
  set(config_header
      "${CMAKE_CURRENT_BINARY_DIR}/include/hpx/${modulename}/config/defines.hpp"
  )
  hpx_local_write_config_defines_file(
    NAMESPACE ${modulename_upper} FILENAME ${config_header}
  )
  set(generated_headers ${generated_headers} ${config_header})

  if(${modulename}_CONFIG_FILES)
    # Version file
    set(global_config_file
        ${CMAKE_CURRENT_BINARY_DIR}/include/hpx/local/config/version.hpp
    )
    configure_file(
      "${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/config_version.hpp.in"
      "${global_config_file}" @ONLY
    )
    set(generated_headers ${generated_headers} ${global_config_file})
    # Global config defines file (different from the one for each module)
    set(global_config_file
        ${CMAKE_CURRENT_BINARY_DIR}/include/hpx/local/config/defines.hpp
    )
    hpx_local_write_config_defines_file(
      TEMPLATE
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/config_defines.hpp.in"
      NAMESPACE default
      FILENAME "${global_config_file}"
    )
    set(generated_headers ${generated_headers} ${global_config_file})
  endif()

  # collect zombie generated headers
  file(GLOB_RECURSE zombie_generated_headers
       ${CMAKE_CURRENT_BINARY_DIR}/include/*.hpp
       ${CMAKE_CURRENT_BINARY_DIR}/include_compatibility/*.hpp
  )
  list(REMOVE_ITEM zombie_generated_headers ${generated_headers}
       ${compat_headers}
       ${CMAKE_CURRENT_BINARY_DIR}/include/hpx/local/config/modules_enabled.hpp
  )
  foreach(zombie_header IN LISTS zombie_generated_headers)
    hpx_local_warn("Removing zombie generated header: ${zombie_header}")
    file(REMOVE ${zombie_header})
  endforeach()

  # list all specified headers
  foreach(header_file ${headers})
    hpx_local_debug(${header_file})
  endforeach(header_file)

  if (${modulename}_SOURCES)
    set(module_library_type OBJECT)
    set(module_public_keyword PUBLIC)
    set(module_private_keyword PRIVATE)
  else()
    set(module_library_type INTERFACE)
    set(module_public_keyword INTERFACE)
    set(module_private_keyword INTERFACE)
  endif()

  # create library modules
  add_library(
    hpx_${modulename}
    ${module_library_type}
    ${sources}
    ${${modulename}_OBJECTS}
    ${headers}
    ${generated_headers}
    ${compat_headers}
  )

  if(HPXLocal_WITH_CHECK_MODULE_DEPENDENCIES)
    # verify that all dependencies are from the same module category
    foreach(dep ${${modulename}_MODULE_DEPENDENCIES})
      # consider only module dependencies, not other targets
      string(FIND ${dep} "hpx_" find_index)
      if(${find_index} EQUAL 0)
        string(SUBSTRING ${dep} 4 -1 dep) # cut off leading "hpx_"
        list(FIND _hpx_${libname}_modules ${dep} dep_index)
        if(${dep_index} EQUAL -1)
          hpx_local_error(
            "The module ${dep} should not be be listed in MODULE_DEPENDENCIES "
            "for module hpx_${modulename}"
          )
        endif()
      endif()
    endforeach()
  endif()

  target_link_libraries(
    hpx_${modulename} ${module_public_keyword} ${${modulename}_MODULE_DEPENDENCIES}
  )
  target_link_libraries(hpx_${modulename} ${module_public_keyword} ${${modulename}_DEPENDENCIES})
  target_include_directories(
    hpx_${modulename}
    ${module_public_keyword} $<BUILD_INTERFACE:${HEADER_ROOT}>
           $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
           $<INSTALL_INTERFACE:include>
  )

  target_link_libraries(
    hpx_${modulename}
    ${module_public_keyword} hpx_local_public_flags
    ${module_private_keyword} hpx_local_private_flags
    ${module_public_keyword} hpx_local_base_libraries
  )

  if(HPXLocal_WITH_PRECOMPILED_HEADERS)
    target_precompile_headers(
      hpx_${modulename} REUSE_FROM hpx_local_precompiled_headers
    )
  endif()

  if(${modulename}_COMPAT_HEADERS)
    target_include_directories(
      hpx_${modulename} ${module_public_keyword} $<BUILD_INTERFACE:${COMPAT_HEADER_ROOT}>
    )
  endif()

  target_compile_definitions(
    hpx_${modulename} ${module_private_keyword} HPX_${libname_upper}_EXPORTS
  )

  target_include_directories(
    hpx_${modulename} ${module_public_keyword} $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
                             $<INSTALL_INTERFACE:include>
  )

  hpx_local_add_source_group(
    NAME hpx_${modulename}
    ROOT ${HEADER_ROOT}/hpx
    CLASS "Header Files"
    TARGETS ${headers}
  )
  hpx_local_add_source_group(
    NAME hpx_${modulename}
    ROOT ${SOURCE_ROOT}
    CLASS "Source Files"
    TARGETS ${sources}
  )
  if(${modulename}_COMPAT_HEADERS)
    hpx_local_add_source_group(
      NAME hpx_${modulename}
      ROOT ${COMPAT_HEADER_ROOT}/hpx
      CLASS "Generated Files"
      TARGETS ${compat_headers}
    )
  endif()

  if(${modulename}_GLOBAL_HEADER_GEN OR ${modulename}_CONFIG_FILES)
    hpx_local_add_source_group(
      NAME hpx_${modulename}
      ROOT ${CMAKE_CURRENT_BINARY_DIR}/include/hpx
      CLASS "Generated Files"
      TARGETS ${generated_headers}
    )
  endif()
  hpx_local_add_source_group(
    NAME hpx_${modulename}
    ROOT ${CMAKE_CURRENT_BINARY_DIR}/include/hpx
    CLASS "Generated Files"
    TARGETS ${config_header}
  )

  # capitalize string
  string(SUBSTRING ${libname} 0 1 first_letter)
  string(TOUPPER ${first_letter} first_letter)
  string(REGEX REPLACE "^.(.*)" "${first_letter}\\1" libname_cap "${libname}")

  set_target_properties(
    hpx_${modulename} PROPERTIES FOLDER "Core/Modules/${libname_cap}"
                                 POSITION_INDEPENDENT_CODE ON
  )

  if(HPXLocal_WITH_UNITY_BUILD)
    set_target_properties(hpx_${modulename} PROPERTIES UNITY_BUILD ON)
  endif()

  if(MSVC)
    set_target_properties(
      hpx_${modulename}
      PROPERTIES COMPILE_PDB_NAME_DEBUG hpx_${modulename}d
                 COMPILE_PDB_NAME_RELWITHDEBINFO hpx_${modulename}
                 COMPILE_PDB_OUTPUT_DIRECTORY_DEBUG
                 ${CMAKE_CURRENT_BINARY_DIR}/Debug
                 COMPILE_PDB_OUTPUT_DIRECTORY_RELWITHDEBINFO
                 ${CMAKE_CURRENT_BINARY_DIR}/RelWithDebInfo
    )
  endif()

  install(
    TARGETS hpx_${modulename}
    EXPORT HPXLocalInternalTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT ${modulename}
  )
  hpx_local_export_internal_targets(hpx_${modulename})

  # Install the headers from the source
  install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT ${modulename}
  )

  # Install the compatibility headers from the source
  if(${modulename}_COMPAT_HEADERS)
    install(
      DIRECTORY ${COMPAT_HEADER_ROOT}/hpx
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
      COMPONENT ${modulename}
    )
  endif()

  # Installing the generated header files from the build dir
  install(
    DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/hpx
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT ${modulename}
  )

  # install PDB if needed
  if(MSVC)
    foreach(cfg DEBUG;RELWITHDEBINFO)
      hpx_local_get_target_property(
        _pdb_file hpx_${modulename} COMPILE_PDB_NAME_${cfg}
      )
      hpx_local_get_target_property(
        _pdb_dir hpx_${modulename} COMPILE_PDB_OUTPUT_DIRECTORY_${cfg}
      )
      install(
        FILES ${_pdb_dir}/${_pdb_file}.pdb
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        CONFIGURATIONS ${cfg}
        OPTIONAL
      )
    endforeach()
  endif()

  # Link modules to their higher-level libraries
  target_link_libraries(hpx_${libname} PUBLIC hpx_${modulename})
  target_link_libraries(hpx_${libname} PRIVATE ${${modulename}_OBJECTS})

  foreach(dir ${${modulename}_CMAKE_SUBDIRS})
    add_subdirectory(${dir})
  endforeach(dir)

  include(HPXLocal_PrintSummary)
  hpx_local_create_configuration_summary(
    "    Module configuration (${modulename}):" "${modulename}"
  )

endfunction(hpx_local_add_module)
