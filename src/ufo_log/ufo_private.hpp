/*
The BSD 3-clause license
--------------------------------------------------------------------------------
Copyright (c) 2014 Rafael Gago Castano. All rights reserved.

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
#include <ufo_log/decltype_wrap.hpp>

#if defined (UFO_HAS_CONSTEXPR) && defined (UFO_HAS_VARIADIC_TEMPLATES)
    #define UFO_COMPILE_TIME_FMT_CHECK
#endif

#define UFO_GET_ARG1_PRIVATE(arg, ...) arg
#define UFO_GET_FMT_STR_PRIVATE(...) UFO_GET_ARG1_PRIVATE(__VA_ARGS__, dummy)

#ifdef UFO_COMPILE_TIME_FMT_CHECK

#include <ufo_log/decltype_wrap.hpp>
#include <ufo_log/compile_format_validator.hpp>

#define UFO_FMT_STRING_CHECK(...)\
        ufo::trigger_format_error<\
            ufo::fmt_validator::execute<\
                UFO_DECLTYPE_WRAP (__VA_ARGS__)\
                    > (UFO_GET_FMT_STR_PRIVATE (__VA_ARGS__))>()

#else //Microsoft crippled mode

namespace ufo { namespace macro {

template <unsigned N>
inline bool is_literal (const char (&arr)[N])
{
    return true;
}

inline bool is_literal (...)
{
    static_assert (true, "format string is not a compile time literal");
    return false;
}

}} //ufo macro

#define UFO_FMT_STRING_CHECK(...)\
    ::ufo::macro::is_literal (UFO_GET_FMT_STR_PRIVATE (__VA_ARGS__))

#endif

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
        (UFO_FMT_STRING_CHECK (__VA_ARGS__)) &&\
        (instance.min_severity() <= ::ufo::sev::severity_),\
        ::ufo::new_entry(\
            instance, ::ufo::sev::severity_, __VA_ARGS__\
            ))

#define UFO_LOG_TO_STR_PRIVATE(a) #a
#define UFO_LOG_FILELINE_CONCAT_PRIVATE(file, lin)\
    "(" file " :" UFO_LOG_TO_STR_PRIVATE (lin) ") "

#endif /* UFO_PRIVATE_HPP_ */
