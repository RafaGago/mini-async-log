/*
 * log_include.hpp
 *
 *  Created on: Nov 15, 2014
 *      Author: rafgag
 */


#ifndef UFO_LOG_LOG_INCLUDE_HPP_
#define UFO_LOG_LOG_INCLUDE_HPP_

#include <ufo_log/ufo_private.hpp>
#include <ufo_log/ufo_interface.hpp>
#include <ufo_log/util/variadic_macro_arg_count.hpp>

//------------------------------------------------------------------------------
#ifndef UFO_GET_LOGGER_INSTANCE_FUNCNAME
    #define UFO_GET_LOGGER_INSTANCE_FUNC get_ufo_logger_instance()
#else
    #define UFO_GET_LOGGER_INSTANCE_FUNC UFO_GET_LOGGER_INSTANCE_FUNCNAME()
#endif
//------------------------------------------------------------------------------
#define log_if(condition, statement) UFO_LOG_IF_PRIVATE (condition, statement)
//------------------------------------------------------------------------------
#define log_every(count, statement) \
    UFO_LOG_EVERY_PRIVATE (__LINE__, count, statement)
//------------------------------------------------------------------------------
#ifndef UFO_STRIP_LOG_FILELINE
    #define log_fileline UFO_LOG_FILELINE_CONCAT_PRIVATE (__FILE__, __LINE__)
#else
    #define log_fileline
#endif
//------------------------------------------------------------------------------
#define log_fln log_fileline
//------------------------------------------------------------------------------
#if !defined (UFO_STRIP_LOG_SEVERITY)

#define log_debug_i(instance, ...)\
    UFO_LOG_PRIVATE (instance, debug, __VA_ARGS__)

#define log_debug(...)\
    log_debug_i (UFO_GET_LOGGER_INSTANCE_FUNC, __VA_ARGS__)

#else

    #define log_debug_i(...) ::ufo::macro::silence_warnings()
    #define log_debug(...)   ::ufo::macro::silence_warnings()

#endif //UFO_STRIP_LOG_SEVERITY
//------------------------------------------------------------------------------
#if !defined (UFO_STRIP_LOG_SEVERITY) || UFO_STRIP_LOG_SEVERITY < 1

#define log_trace_i(instance, ...)\
    UFO_LOG_PRIVATE (instance, trace, __VA_ARGS__)

#define log_trace(...)\
    log_trace_i (UFO_GET_LOGGER_INSTANCE_FUNC, __VA_ARGS__)

#else

    #define log_trace_i(...) ::ufo::macro::silence_warnings()
    #define log_trace(...)   ::ufo::macro::silence_warnings()

#endif
//------------------------------------------------------------------------------
#if !defined (UFO_STRIP_LOG_SEVERITY) || UFO_STRIP_LOG_SEVERITY < 2

#define log_notice_i(instance, ...)\
    UFO_LOG_PRIVATE (instance, notice, __VA_ARGS__)

#define log_notice(...)\
    log_notice_i (UFO_GET_LOGGER_INSTANCE_FUNC, __VA_ARGS__)

#else

    #define log_notice_i(...) ::ufo::macro::silence_warnings()
    #define log_notice(...)   ::ufo::macro::silence_warnings()

#endif
//------------------------------------------------------------------------------
#if !defined (UFO_STRIP_LOG_SEVERITY) || UFO_STRIP_LOG_SEVERITY < 3

#define log_warning_i(instance, ...)\
    UFO_LOG_PRIVATE (instance, warning, __VA_ARGS__)

#define log_warning(...)\
    log_warning_i (UFO_GET_LOGGER_INSTANCE_FUNC, __VA_ARGS__)

#else

    #define log_warning_i(...) ::ufo::macro::silence_warnings()
    #define log_warning(...)   ::ufo::macro::silence_warnings()

#endif
//------------------------------------------------------------------------------
#if !defined (UFO_STRIP_LOG_SEVERITY) || UFO_STRIP_LOG_SEVERITY < 4

#define log_error_i(instance, ...)\
    UFO_LOG_PRIVATE (instance, error, __VA_ARGS__)

#define log_error(...)\
    log_error_i (UFO_GET_LOGGER_INSTANCE_FUNC, __VA_ARGS__)

#else

    #define log_error_i(...) ::ufo::macro::silence_warnings()
    #define log_error(...)   ::ufo::macro::silence_warnings()

#endif
//------------------------------------------------------------------------------
#if !defined (UFO_STRIP_LOG_SEVERITY) || UFO_STRIP_LOG_SEVERITY < 5

#define log_critical_i(instance, ...)\
    UFO_LOG_PRIVATE (instance, critical, __VA_ARGS__)

#define log_critical(...)\
    log_critical_i (UFO_GET_LOGGER_INSTANCE_FUNC, __VA_ARGS__)

#else

    #define log_critical_i(...) ::ufo::macro::silence_warnings()
    #define log_critical(...)   ::ufo::macro::silence_warnings()

#endif
//------------------------------------------------------------------------------
#endif /* UFO_LOG_LOG_INCLUDE_HPP_ */
