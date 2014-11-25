/*
The BSD 3-clause license
--------------------------------------------------------------------------------
Copyright (c) 2013-2014 Rafael Gago Castano. All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

   3. Neither the name of the copyright holder nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY RAFAEL GAGO CASTANO "AS IS" AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
SHALL RAFAEL GAGO CASTANO OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of Rafael Gago Castano.
--------------------------------------------------------------------------------
*/

#ifndef UFO_LOG_LOG_INCLUDE_PRIVATE_HPP_
#define UFO_LOG_LOG_INCLUDE_PRIVATE_HPP_

#include <ufo_log/util/system.hpp>

namespace ufo { namespace macro {

template <unsigned count>
struct log_every_needs_values_greater_than
{
    static const unsigned value = count - 1;
};
template <> struct log_every_needs_values_greater_than<0> {};                   //functions logging every n can't be used with values smaller than 2
template <> struct log_every_needs_values_greater_than<1> {};                   //functions logging every n can't be used with values smaller than 2

inline bool silence_warnings() { return true; }

}} //namespaces

#ifdef UFO_HAS_CONSTEXPR

template <unsigned N>
constexpr unsigned ufo_fmt_string_arity(
        const char (&arr)[N], unsigned i = N - 1
        )
{
    static_assert (N > 1, "not a literal");
    return (unsigned) (arr[i] == '{' && arr[i + 1] == '}') +
            ((i != 0) ? ufo_fmt_string_arity (arr, i - 1) : 0);
}

namespace ufo { namespace detail {

template <unsigned real_arity, unsigned string_arity>
constexpr bool ufo_fmt_check()
{
    static_assert(
        real_arity == string_arity,
        "arity mismatch between the format literal and the number of parameters"
        );
    return true;
}

}} //namespace ufo detail

#else //UFO_HAS_CONSTEXPR

#define ufo_fmt_string_arity(dummy) 0

namespace ufo { namespace detail {

template <unsigned real_arity, unsigned string_arity>
inline bool ufo_fmt_check()
{
    return true;
}

}} //namespace ufo detail

#endif //UFO_HAS_CONSTEXPR

#define UFO_GET_ARG1_PRIVATE(arg, ...) arg
#define UFO_GET_FMT_STR_PRIVATE(...) UFO_GET_ARG1_PRIVATE(__VA_ARGS__, dummy)

#define UFO_LOG_IF_PRIVATE(condition, statement)\
    ((condition) ? (statement) : ::ufo::macro::silence_warnings())

#define UFO_LOG_EVERY_PRIVATE(line, count, statement)\
    ([&]() -> bool \
    { \
        static unsigned counter##line = \
            ::ufo::macro::log_every_needs_values_greater_than<count>::value; \
        ++counter##line = (counter##line != count) ? counter##line : 0; \
        return UFO_LOG_IF_PRIVATE (counter##line == 0, statement); \
    }.operator ()());

#define UFO_LOG_PRIVATE(instance, severity_, ...)\
    UFO_LOG_IF_PRIVATE(\
        (ufo::detail::ufo_fmt_check<\
            (UFO_COUNT_VA_ARGS (__VA_ARGS__) - 1),\
            ufo_fmt_string_arity (UFO_GET_FMT_STR_PRIVATE (__VA_ARGS__))\
            >() &&\
        (instance.severity() <= ::ufo::sev::severity_)),\
        ::ufo::new_entry(\
            instance, ::ufo::sev::severity_, __VA_ARGS__\
            ))

#define UFO_LOG_TO_STR_PRIVATE(a) #a
#define UFO_LOG_FILELINE_CONCAT_PRIVATE(file, lin)\
    "(" file " :" UFO_LOG_TO_STR_PRIVATE (lin) ") "

#endif /* UFO_PRIVATE_HPP_ */
