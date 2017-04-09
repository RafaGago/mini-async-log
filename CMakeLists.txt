cmake_minimum_required(VERSION 2.8.5 FATAL_ERROR)
project(mini-async-log)

option(USE_BOOST "Use Boost atomic, chrono, cstdint and thread" OFF)
option(STRIP_LOG_DEBUG "Remove this log level and levels less severe during compilation" OFF)
option(STRIP_LOG_TRACE "Remove this log level and levels less severe during compilation" OFF)
option(STRIP_LOG_NOTICE "Remove this log level and levels less severe during compilation" OFF)
option(STRIP_LOG_WARNING "Remove this log level and levels less severe during compilation" OFF)
option(STRIP_LOG_ERROR "Remove this log level and levels less severe during compilation" OFF)
option(STRIP_LOG_CRITICAL "Remove all log levels during compilation" OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CLANG_OR_GCC 1)
else()
    set(CLANG_OR_GCC 0)
endif()

if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CLANG_OR_GCC)
      set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
    endif ()
else ()
    set(CMAKE_CXX_STANDARD 11)
endif ()

if(CLANG_OR_GCC)
    set(CMAKE_REQUIRED_DEFINITIONS "-std=c++11")
endif()

include(CheckIncludeFileCXX)

if(USE_BOOST)
    find_package(Boost REQUIRED COMPONENTS atomic chrono thread)

    if(Boost_FOUND)
        include_directories(${Boost_INCLUDE_DIRS})
    else()
        message(FATAL_ERROR "Boost not found")
    endif()
else()
    check_include_file_cxx(thread HAVE_STD_THREAD)
    check_include_file_cxx(atomic HAVE_STD_ATOMIC)
    check_include_file_cxx(chrono HAVE_STD_CHRONO)
    check_include_file_cxx(cstdint HAVE_STD_CSTDINT)

    if(NOT HAVE_STD_CSTDINT OR NOT HAVE_STD_ATOMIC OR NOT HAVE_STD_CHRONO OR NOT HAVE_STD_THREAD)
        message(FATAL_ERROR "<atomic>, <cstdint>, <chrono>, and/or <thread> not found")
    endif()
endif()

include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/src")

set(mal_PUBLIC_HEADERS
    "${PROJECT_SOURCE_DIR}/include/mal_log/cfg.hpp"
    "${PROJECT_SOURCE_DIR}/include/mal_log/compile_format_validator.hpp"
    "${PROJECT_SOURCE_DIR}/include/mal_log/decltype_wrap.hpp"
    "${PROJECT_SOURCE_DIR}/include/mal_log/extras/boost_filesystem_list_all_files.hpp"
    "${PROJECT_SOURCE_DIR}/include/mal_log/format_tokens.hpp"
    "${PROJECT_SOURCE_DIR}/include/mal_log/frontend.hpp"
    "${PROJECT_SOURCE_DIR}/include/mal_log/frontend_types.hpp"
    "${PROJECT_SOURCE_DIR}/include/mal_log/frontend_types_convenience_funcs_to_current_namespace.hpp"
    "${PROJECT_SOURCE_DIR}/include/mal_log/mal_interface.hpp"
    "${PROJECT_SOURCE_DIR}/include/mal_log/mal_log.hpp"
    "${PROJECT_SOURCE_DIR}/include/mal_log/mal_private.hpp"
    "${PROJECT_SOURCE_DIR}/include/mal_log/mal_strip.hpp"
    "${PROJECT_SOURCE_DIR}/include/mal_log/serialization/basic_encoder_decoder.hpp"
    "${PROJECT_SOURCE_DIR}/include/mal_log/serialization/exporter.hpp"
    "${PROJECT_SOURCE_DIR}/include/mal_log/serialization/fields.hpp"
    "${PROJECT_SOURCE_DIR}/include/mal_log/serialization/header_data.hpp"
    "${PROJECT_SOURCE_DIR}/include/mal_log/serialization/importer_exporter.hpp"
    "${PROJECT_SOURCE_DIR}/include/mal_log/sync_point.hpp"
    "${PROJECT_SOURCE_DIR}/include/mal_log/timestamp.hpp"
    "${PROJECT_SOURCE_DIR}/include/mal_log/util/atomic.hpp"
    "${PROJECT_SOURCE_DIR}/include/mal_log/util/chrono.hpp"
    "${PROJECT_SOURCE_DIR}/include/mal_log/util/integer.hpp"
    "${PROJECT_SOURCE_DIR}/include/mal_log/util/integer_bits.hpp"
    "${PROJECT_SOURCE_DIR}/include/mal_log/util/integer_bits_detail.hpp"
    "${PROJECT_SOURCE_DIR}/include/mal_log/util/integral_enable_if.hpp"
    "${PROJECT_SOURCE_DIR}/include/mal_log/util/literal.hpp"
    "${PROJECT_SOURCE_DIR}/include/mal_log/util/max_align.hpp"
    "${PROJECT_SOURCE_DIR}/include/mal_log/util/opaque_pod.hpp"
    "${PROJECT_SOURCE_DIR}/include/mal_log/util/processor_pause.hpp"
    "${PROJECT_SOURCE_DIR}/include/mal_log/util/queue_backoff_cfg.hpp"
    "${PROJECT_SOURCE_DIR}/include/mal_log/util/side_effect_assert.hpp"
    "${PROJECT_SOURCE_DIR}/include/mal_log/util/stack_ostream.hpp"
    "${PROJECT_SOURCE_DIR}/include/mal_log/util/static_rdbuf.hpp"
    "${PROJECT_SOURCE_DIR}/include/mal_log/util/system.hpp"
    "${PROJECT_SOURCE_DIR}/include/mal_log/util/thread.hpp"
    "${PROJECT_SOURCE_DIR}/include/mal_log/util/variadic_macro_arg_count.hpp"
)

set(mal_PRIVATE_HEADERS
    "${PROJECT_SOURCE_DIR}/src/mal_log/async_to_sync.hpp"
    "${PROJECT_SOURCE_DIR}/src/mal_log/backend.hpp"
    "${PROJECT_SOURCE_DIR}/src/mal_log/backoff_wait.hpp"
    "${PROJECT_SOURCE_DIR}/src/mal_log/log_file_register.hpp"
    "${PROJECT_SOURCE_DIR}/src/mal_log/log_writer.hpp"
    "${PROJECT_SOURCE_DIR}/src/mal_log/output.hpp"
    "${PROJECT_SOURCE_DIR}/src/mal_log/queue.hpp"
    "${PROJECT_SOURCE_DIR}/src/mal_log/serialization/byte_stream_convert.hpp"
    "${PROJECT_SOURCE_DIR}/src/mal_log/serialization/importer.hpp"
    "${PROJECT_SOURCE_DIR}/src/mal_log/serialization/printf_modifiers.hpp"
    "${PROJECT_SOURCE_DIR}/src/mal_log/util/aligned_type.hpp"
    "${PROJECT_SOURCE_DIR}/src/mal_log/util/calendar_str.hpp"
    "${PROJECT_SOURCE_DIR}/src/mal_log/util/memory_mpmc_bounded.hpp"
    "${PROJECT_SOURCE_DIR}/src/mal_log/util/mem_printf.hpp"
    "${PROJECT_SOURCE_DIR}/src/mal_log/util/mpmc_bounded.hpp"
    "${PROJECT_SOURCE_DIR}/src/mal_log/util/mpsc.hpp"
    "${PROJECT_SOURCE_DIR}/src/mal_log/util/on_stack_dynamic.hpp"
    "${PROJECT_SOURCE_DIR}/src/mal_log/util/placement_new.hpp"
    "${PROJECT_SOURCE_DIR}/src/mal_log/util/queue_backoff.hpp"
    "${PROJECT_SOURCE_DIR}/src/mal_log/util/raw_circular_buffer.hpp"
    "${PROJECT_SOURCE_DIR}/src/mal_log/util/safe_bool.hpp"
)

set(mal_SOURCES
    "${PROJECT_SOURCE_DIR}/src/mal_log/frontend.cpp"
)

if(STRIP_LOG_DEBUG)
    add_definitions(-DMAL_STRIP_LOG_SEVERITY=0)
elseif(STRIP_LOG_TRACE)
    add_definitions(-DMAL_STRIP_LOG_SEVERITY=1)
elseif(STRIP_LOG_NOTICE)
    add_definitions(-DMAL_STRIP_LOG_SEVERITY=2)
elseif(STRIP_LOG_WARNING)
    add_definitions(-DMAL_STRIP_LOG_SEVERITY=3)
elseif(STRIP_LOG_ERROR)
    add_definitions(-DMAL_STRIP_LOG_SEVERITY=4)
elseif(STRIP_LOG_CRITICAL)
    add_definitions(-DMAL_STRIP_LOG_SEVERITY=5)
endif()

if(BUILD_SHARED_LIBS)
    add_definitions(-DMAL_DYNLIB_COMPILE)
endif()

if(USE_BOOST)
    add_definitions(-DMAL_USE_BOOST_ATOMIC -DMAL_USE_BOOST_CHRONO -DMAL_USE_BOOST_CSTDINT -DMAL_USE_BOOST_THREAD)
endif()

add_library(mini_async_log ${mal_SOURCES})

if(USE_BOOST)
    target_link_libraries(mini_async_log ${Boost_LIBRARIES})
endif()

install(TARGETS mini_async_log
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_PREFIX} FILES_MATCHING PATTERN "*.hpp")
install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_PREFIX} FILES_MATCHING PATTERN "*.h")
