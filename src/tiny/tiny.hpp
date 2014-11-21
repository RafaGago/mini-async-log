/*
 * log_include.hpp
 *
 *  Created on: Nov 15, 2014
 *      Author: rafgag
 */


#ifndef TINY_LOG_LOG_INCLUDE_HPP_
#define TINY_LOG_LOG_INCLUDE_HPP_

#include <tiny/tiny_interface.hpp>

#ifndef TINY_GET_LOGGER_INSTANCE_FUNC
    #define TINY_GET_LOGGER_INSTANCE_FUNC get_tiny_logger_instance()
#endif

namespace tiny { namespace macro {

template <unsigned count>
struct log_every_needs_values_greater_than
{
    static const unsigned value = count - 1;
};
template <> struct log_every_needs_values_greater_than<0> {}; //functions logging every n can't be used with values smaller than 2
template <> struct log_every_needs_values_greater_than<1> {}; //functions logging every n can't be used with values smaller than 2

inline bool silence_warnings() { return true; }

}}

#define log_if(condition, statement)\
    ((condition) ? (statement) : ::tiny::macro::silence_warnings())

#define LOG_EVERY_WRAP_EXPANDED_NOUSE(line, count, statement)\
    ([&]() -> bool \
    { \
        namespace mu = ::tiny::macro;\
        static unsigned counter##line = \
        mu::log_every_needs_values_greater_than<count>::value; \
        ++counter##line = (counter##line != count) ? counter##line : 0; \
        return log_if (counter##line == 0, statement); \
    }.operator ()());

#define LOG_NOUSE(instance, severity_, ...)\
    log_if(\
        (instance.severity() <= ::tiny::sev::severity_),\
        ::tiny::new_entry(\
            instance, ::tiny::sev::severity_, __VA_ARGS__\
            ))

#define LOG_TO_STR_NOUSE(a) #a
#define LOG_FILELINE_CONCAT_NOUSE(file, lin)\
    "(" file " :" LOG_TO_STR_NOUSE (lin) ") "

#ifndef TINY_STRIP_LOG_FILELINE
    #define log_fileline LOG_FILELINE_CONCAT_NOUSE (__FILE__, __LINE__)
    #define log_fln      log_fileline
#else
    #define log_fileline
    #define log_fln      log_fileline
#endif

#define log_every(count, statement) \
    LOG_EVERY_WRAP_EXPANDED_NOUSE(__LINE__, count, statement)
//------------------------------------------------------------------------------
#if !defined (TINY_STRIP_LOG_SEVERITY)

#define log_debug_i(instance, ...)\
    LOG_NOUSE (instance, debug, __VA_ARGS__)

#define log_debug(...)\
    log_debug_i (TINY_GET_LOGGER_INSTANCE_FUNC, __VA_ARGS__)

#else

    #define log_debug_i(...) ::tiny::macro::silence_warnings()
    #define log_debug(...)   ::tiny::macro::silence_warnings()

#endif //TINY_STRIP_LOG_SEVERITY
//------------------------------------------------------------------------------
#if !defined (TINY_STRIP_LOG_SEVERITY) || TINY_STRIP_LOG_SEVERITY < 1

#define log_trace_i(instance, ...)\
    LOG_NOUSE (instance, trace, __VA_ARGS__)

#define log_trace(...)\
    log_trace_i (TINY_GET_LOGGER_INSTANCE_FUNC, __VA_ARGS__)

#else

    #define log_trace_i(...) ::tiny::macro::silence_warnings()
    #define log_trace(...)   ::tiny::macro::silence_warnings()

#endif
//------------------------------------------------------------------------------
#if !defined (TINY_STRIP_LOG_SEVERITY) || TINY_STRIP_LOG_SEVERITY < 2

#define log_notice_i(instance, ...)\
    LOG_NOUSE (instance, notice, __VA_ARGS__)

#define log_notice(...)\
    log_notice_i (TINY_GET_LOGGER_INSTANCE_FUNC, __VA_ARGS__)

#else

    #define log_notice_i(...) ::tiny::macro::silence_warnings()
    #define log_notice(...)   ::tiny::macro::silence_warnings()

#endif
//------------------------------------------------------------------------------
#if !defined (TINY_STRIP_LOG_SEVERITY) || TINY_STRIP_LOG_SEVERITY < 3

#define log_warning_i(instance, ...)\
    LOG_NOUSE (instance, warning, __VA_ARGS__)

#define log_warning(...)\
    log_warning_i (TINY_GET_LOGGER_INSTANCE_FUNC, __VA_ARGS__)

#else

    #define log_warning_i(...) ::tiny::macro::silence_warnings()
    #define log_warning(...)   ::tiny::macro::silence_warnings()

#endif
//------------------------------------------------------------------------------
#if !defined (TINY_STRIP_LOG_SEVERITY) || TINY_STRIP_LOG_SEVERITY < 4

#define log_error_i(instance, ...)\
    LOG_NOUSE (instance, error, __VA_ARGS__)

#define log_error(...)\
    log_error_i (TINY_GET_LOGGER_INSTANCE_FUNC, __VA_ARGS__)

#else

    #define log_error_i(...) ::tiny::macro::silence_warnings()
    #define log_error(...)   ::tiny::macro::silence_warnings()

#endif
//------------------------------------------------------------------------------
#if !defined (TINY_STRIP_LOG_SEVERITY) || TINY_STRIP_LOG_SEVERITY < 5

#define log_critical_i(instance, ...)\
    LOG_NOUSE (instance, critical, __VA_ARGS__)

#define log_critical(...)\
    log_critical_i (TINY_GET_LOGGER_INSTANCE_FUNC, __VA_ARGS__)

#else

    #define log_critical_i(...) ::tiny::macro::silence_warnings()
    #define log_critical(...)   ::tiny::macro::silence_warnings()

#endif
//------------------------------------------------------------------------------
#endif /* TINY_LOG_LOG_INCLUDE_HPP_ */
