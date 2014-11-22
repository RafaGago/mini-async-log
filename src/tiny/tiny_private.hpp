/*
 * tiny_private.hpp
 *
 *  Created on: Nov 22, 2014
 *      Author: rafa
 */

#ifndef TINY_LOG_LOG_INCLUDE_PRIVATE_HPP_
#define TINY_LOG_LOG_INCLUDE_PRIVATE_HPP_

#include <tiny/util/system.hpp>

namespace tiny { namespace macro {

template <unsigned count>
struct log_every_needs_values_greater_than
{
    static const unsigned value = count - 1;
};
template <> struct log_every_needs_values_greater_than<0> {};                   //functions logging every n can't be used with values smaller than 2
template <> struct log_every_needs_values_greater_than<1> {};                   //functions logging every n can't be used with values smaller than 2

inline bool silence_warnings() { return true; }

}} //namespaces

#ifdef TINY_HAS_CONSTEXPR

template <unsigned N>
constexpr unsigned tiny_fmt_string_arity(
        const char (&arr)[N], unsigned i = N - 1
        )
{
    static_assert (N > 1, "not a literal");
    return (unsigned) (arr[i] == '{' && arr[i + 1] == '}') +
            ((i != 0) ? tiny_fmt_string_arity (arr, i - 1) : 0);
}

namespace tiny { namespace detail {

template <unsigned real_arity, unsigned string_arity>
constexpr bool tiny_fmt_check()
{
    static_assert(
        real_arity == string_arity,
        "arity mismatch between the format literal and the number of parameters"
        );
    return true;
}

}} //namespace tiny detail

#else //TINY_HAS_CONSTEXPR

#define tiny_fmt_string_arity(dummy) 0

namespace tiny { namespace detail {

template <unsigned real_arity, unsigned string_arity>
inline bool tiny_fmt_check()
{
    return true;
}

}} //namespace tiny detail

#endif //TINY_HAS_CONSTEXPR

#define TINY_GET_ARG1_PRIVATE(arg, ...) arg
#define TINY_GET_FMT_STR_PRIVATE(...) TINY_GET_ARG1_PRIVATE(__VA_ARGS__, dummy)

#define TINY_LOG_IF_PRIVATE(condition, statement)\
    ((condition) ? (statement) : ::tiny::macro::silence_warnings())

#define TINY_LOG_EVERY_PRIVATE(line, count, statement)\
    ([&]() -> bool \
    { \
        static unsigned counter##line = \
            ::tiny::macro::log_every_needs_values_greater_than<count>::value; \
        ++counter##line = (counter##line != count) ? counter##line : 0; \
        return TINY_LOG_IF_PRIVATE (counter##line == 0, statement); \
    }.operator ()());

#define TINY_LOG_PRIVATE(instance, severity_, ...)\
    TINY_LOG_IF_PRIVATE(\
        (tiny::detail::tiny_fmt_check<\
            (TINY_COUNT_VA_ARGS (__VA_ARGS__) - 1),\
            tiny_fmt_string_arity (TINY_GET_FMT_STR_PRIVATE (__VA_ARGS__))\
            >() &&\
        (instance.severity() <= ::tiny::sev::severity_)),\
        ::tiny::new_entry(\
            instance, ::tiny::sev::severity_, __VA_ARGS__\
            ))

#define TINY_LOG_TO_STR_PRIVATE(a) #a
#define TINY_LOG_FILELINE_CONCAT_PRIVATE(file, lin)\
    "(" file " :" TINY_LOG_TO_STR_PRIVATE (lin) ") "

#endif /* TINY_PRIVATE_HPP_ */
