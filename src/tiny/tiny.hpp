/*
 * log_include.hpp
 *
 *  Created on: Nov 15, 2014
 *      Author: rafgag
 */


#ifndef TINY_LOG_LOG_INCLUDE_HPP_
#define TINY_LOG_LOG_INCLUDE_HPP_

#include <tiny/tiny_private.hpp>
#include <tiny/tiny_interface.hpp>
#include <tiny/util/variadic_macro_arg_count.hpp>

//------------------------------------------------------------------------------
#ifndef TINY_GET_LOGGER_INSTANCE_FUNCNAME
    #define TINY_GET_LOGGER_INSTANCE_FUNC get_tiny_logger_instance()
#else
    #define TINY_GET_LOGGER_INSTANCE_FUNC TINY_GET_LOGGER_INSTANCE_FUNCNAME()
#endif
//------------------------------------------------------------------------------
#define log_if(condition, statement) TINY_LOG_IF_PRIVATE (condition, statement)
//------------------------------------------------------------------------------
#define log_every(count, statement) \
    TINY_LOG_EVERY_PRIVATE (__LINE__, count, statement)
//------------------------------------------------------------------------------
#ifndef TINY_STRIP_LOG_FILELINE
    #define log_fileline TINY_LOG_FILELINE_CONCAT_PRIVATE (__FILE__, __LINE__)
#else
    #define log_fileline
#endif
//------------------------------------------------------------------------------
#define log_fln log_fileline
//------------------------------------------------------------------------------
#if !defined (TINY_STRIP_LOG_SEVERITY)

#define log_debug_i(instance, ...)\
    TINY_LOG_PRIVATE (instance, debug, __VA_ARGS__)

#define log_debug(...)\
    log_debug_i (TINY_GET_LOGGER_INSTANCE_FUNC, __VA_ARGS__)

#else

    #define log_debug_i(...) ::tiny::macro::silence_warnings()
    #define log_debug(...)   ::tiny::macro::silence_warnings()

#endif //TINY_STRIP_LOG_SEVERITY
//------------------------------------------------------------------------------
#if !defined (TINY_STRIP_LOG_SEVERITY) || TINY_STRIP_LOG_SEVERITY < 1

#define log_trace_i(instance, ...)\
    TINY_LOG_PRIVATE (instance, trace, __VA_ARGS__)

#define log_trace(...)\
    log_trace_i (TINY_GET_LOGGER_INSTANCE_FUNC, __VA_ARGS__)

#else

    #define log_trace_i(...) ::tiny::macro::silence_warnings()
    #define log_trace(...)   ::tiny::macro::silence_warnings()

#endif
//------------------------------------------------------------------------------
#if !defined (TINY_STRIP_LOG_SEVERITY) || TINY_STRIP_LOG_SEVERITY < 2

#define log_notice_i(instance, ...)\
    TINY_LOG_PRIVATE (instance, notice, __VA_ARGS__)

#define log_notice(...)\
    log_notice_i (TINY_GET_LOGGER_INSTANCE_FUNC, __VA_ARGS__)

#else

    #define log_notice_i(...) ::tiny::macro::silence_warnings()
    #define log_notice(...)   ::tiny::macro::silence_warnings()

#endif
//------------------------------------------------------------------------------
#if !defined (TINY_STRIP_LOG_SEVERITY) || TINY_STRIP_LOG_SEVERITY < 3

#define log_warning_i(instance, ...)\
    TINY_LOG_PRIVATE (instance, warning, __VA_ARGS__)

#define log_warning(...)\
    log_warning_i (TINY_GET_LOGGER_INSTANCE_FUNC, __VA_ARGS__)

#else

    #define log_warning_i(...) ::tiny::macro::silence_warnings()
    #define log_warning(...)   ::tiny::macro::silence_warnings()

#endif
//------------------------------------------------------------------------------
#if !defined (TINY_STRIP_LOG_SEVERITY) || TINY_STRIP_LOG_SEVERITY < 4

#define log_error_i(instance, ...)\
    TINY_LOG_PRIVATE (instance, error, __VA_ARGS__)

#define log_error(...)\
    log_error_i (TINY_GET_LOGGER_INSTANCE_FUNC, __VA_ARGS__)

#else

    #define log_error_i(...) ::tiny::macro::silence_warnings()
    #define log_error(...)   ::tiny::macro::silence_warnings()

#endif
//------------------------------------------------------------------------------
#if !defined (TINY_STRIP_LOG_SEVERITY) || TINY_STRIP_LOG_SEVERITY < 5

#define log_critical_i(instance, ...)\
    TINY_LOG_PRIVATE (instance, critical, __VA_ARGS__)

#define log_critical(...)\
    log_critical_i (TINY_GET_LOGGER_INSTANCE_FUNC, __VA_ARGS__)

#else

    #define log_critical_i(...) ::tiny::macro::silence_warnings()
    #define log_critical(...)   ::tiny::macro::silence_warnings()

#endif
//------------------------------------------------------------------------------
#endif /* TINY_LOG_LOG_INCLUDE_HPP_ */
