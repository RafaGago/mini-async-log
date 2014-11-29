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

#ifndef UFO_LOG_FORMAT_VALIDATOR_HPP_
#define UFO_LOG_FORMAT_VALIDATOR_HPP_

#include <tuple>
#include <type_traits>
#include <ufo_log/util/integer.hpp>
#include <ufo_log/util/literal.hpp>
#include <ufo_log/format_tokens.hpp>

namespace ufo {

namespace fmt_error {
//------------------------------------------------------------------------------
static const uword pars        = 1 << ((sizeof (uword) * 8) - 1);
static const uword pchs        = 1 << ((sizeof (uword) * 8) - 2);
static const uword modif       = 1 << ((sizeof (uword) * 8) - 3);
static const uword error_mask  = modif - 1;
//------------------------------------------------------------------------------
constexpr bool is_arity (uword arity, uword val)
{
    return (val & error_mask) == arity;
}
//------------------------------------------------------------------------------
constexpr bool parameter (uword arity, uword val)
{
    return !(((val & pars) == pars) && (is_arity (arity, val) || arity == 0));
}
//------------------------------------------------------------------------------
constexpr bool placeholder (uword arity, uword val)
{
    return !(((val & pchs) == pchs) && (is_arity (arity, val) || arity == 0));
}
//------------------------------------------------------------------------------
constexpr bool modifier (uword arity, uword val)
{
    return !(((val & modif) == modif) && (is_arity (arity, val) || arity == 0));
}
//------------------------------------------------------------------------------
} //namespace fmt_error
//------------------------------------------------------------------------------
struct fmt_validator
{
public:
    //--------------------------------------------------------------------------
    template <class... args>
    static constexpr word execute (literal l)
    {
        return scan<args...> (l, 0, 0);
    }
private:

    template <class T>
    struct fmt_check_adapt
    {
        typedef typename std::remove_reference<
                        typename std::remove_cv<T>::type
                        >::type                           type;
    };
    //--------------------------------------------------------------------------
    template <uword dummy = 1>
    struct at_least_one {};
    //--------------------------------------------------------------------------
    template <class T>
    static constexpr typename std::enable_if<
        std::is_integral<T>::value && !std::is_same<T, bool>::value,
        bool
        >::type
    is_formatting_valid (char f, T*)
    {
        return (f == fmt::hex)        ? true :
               (f == fmt::full_width) ? true :
                                        false;
    }
    //--------------------------------------------------------------------------
    template <class T>
    static constexpr typename std::enable_if<
        std::is_floating_point<T>::value,
        bool
        >::type
    is_formatting_valid (char f, T*)
    {
        return (f == fmt::hex)        ? true :
               (f == fmt::scientific) ? true :
                                        false;
    }
    //--------------------------------------------------------------------------
    template <class T>
    static constexpr bool is_formatting_valid (char c, ...)
    {
        return false;
    }
    //--------------------------------------------------------------------------
    static constexpr bool token_with_fmt (literal l, uword i)
    {
        return (i <= (l.size() - 3)) &&
               (l[i]     == fmt::placeholder_open) &&
               (l[i + 2] == fmt::placeholder_close);
    }
    //--------------------------------------------------------------------------
    static constexpr bool token (literal l, uword i)
    {
        return (i <= (l.size() - 2)) &&
               (l[i]     == fmt::placeholder_open) &&
               (l[i + 1] == fmt::placeholder_close);
    }
    //--------------------------------------------------------------------------
    static constexpr bool finished (literal l, uword i)
    {
        return i >= (l.size() - 1);
    }
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    template <class dummy, class T, class... args>
    static constexpr word consume_param_impl (literal l, uword i, uword arity)
    {
        return scan<args...> (l, i + 2, arity + 1);
    }
    //--------------------------------------------------------------------------
    template <class dummy>
    static constexpr word consume_param_impl (literal l, uword i, uword arity)
    {
        return (arity + 1) | fmt_error::pchs;
    }
    //--------------------------------------------------------------------------
    template <class... args>
    static constexpr word consume_param (literal l, uword i, uword arity)
    {
        return consume_param_impl<at_least_one<>, args...> (l, i, arity);
    }
    //--------------------------------------------------------------------------
    template <class dummy, class T, class... args>
    static constexpr word consume_param_with_fmt_impl(
                            literal l, uword i, uword arity
                            )
    {
        typedef typename fmt_check_adapt<T>::type type;
        return is_formatting_valid<type> (l[i + 1], (type*) nullptr) ?
                scan<args...> (l, i + 3, arity + 1) :
                (arity + 1) | fmt_error::modif;
    }
    //--------------------------------------------------------------------------
    template <class dummy>
    static constexpr word consume_param_with_fmt_impl(
                            literal l, uword i, uword arity
                            )
    {
        return (arity + 1) | fmt_error::pchs;
    }
    //--------------------------------------------------------------------------
    template <class... args>
    static constexpr word consume_param_with_fmt(
                            literal l, uword i, uword arity
                            )
    {
        return consume_param_with_fmt_impl<
                                at_least_one<>, args...
                                >(l, i, arity);
    }
    //--------------------------------------------------------------------------
    template <class... args>
    static constexpr word check_arity_match (uword arity)
    {
        return (sizeof... (args) == 0) ? arity : (arity + 1) | fmt_error::pars;
    }
    //--------------------------------------------------------------------------
    template <class... args>
    static constexpr word scan (literal l, uword i, uword arity)
    {
        return (finished (l, i))       ?
                    check_arity_match<args...> (arity)           :
               (token_with_fmt (l, i)) ?
                   consume_param_with_fmt<args...> (l, i, arity) :
               (token (l, i))          ?
                   consume_param<args...> (l, i, arity)          :
                   scan<args...>          (l, i + 1, arity);
    }
    //--------------------------------------------------------------------------
}; //validator
//------------------------------------------------------------------------------
#define UFO_PARAMERR_LIT "too many parameters for format string"
#define UFO_PCHERR_LIT   "too many placeholders in format string"
#define UFO_MODIFERR_LIT "invalid modifier in format string parameter"
//------------------------------------------------------------------------------
template <word result>
constexpr bool trigger_format_error()
{
    static_assert (fmt_error::parameter (1,  result), UFO_PARAMERR_LIT ": 1");
    static_assert (fmt_error::parameter (2,  result), UFO_PARAMERR_LIT ": 2");
    static_assert (fmt_error::parameter (3,  result), UFO_PARAMERR_LIT ": 3");
    static_assert (fmt_error::parameter (4,  result), UFO_PARAMERR_LIT ": 4");
    static_assert (fmt_error::parameter (5,  result), UFO_PARAMERR_LIT ": 5");
    static_assert (fmt_error::parameter (6,  result), UFO_PARAMERR_LIT ": 6");
    static_assert (fmt_error::parameter (7,  result), UFO_PARAMERR_LIT ": 7");
    static_assert (fmt_error::parameter (8,  result), UFO_PARAMERR_LIT ": 8");
    static_assert (fmt_error::parameter (9,  result), UFO_PARAMERR_LIT ": 9");
    static_assert (fmt_error::parameter (10, result), UFO_PARAMERR_LIT ": 10");
    static_assert (fmt_error::parameter (11, result), UFO_PARAMERR_LIT ": 11");
    static_assert (fmt_error::parameter (12, result), UFO_PARAMERR_LIT ": 12");
    static_assert (fmt_error::parameter (13, result), UFO_PARAMERR_LIT ": 13");
    static_assert (fmt_error::parameter (14, result), UFO_PARAMERR_LIT ": 14");
    static_assert (fmt_error::parameter (15, result), UFO_PARAMERR_LIT ": 15");
    static_assert (fmt_error::parameter (16, result), UFO_PARAMERR_LIT ": 16");
    static_assert (fmt_error::parameter (17, result), UFO_PARAMERR_LIT ": 17");
    static_assert (fmt_error::parameter (18, result), UFO_PARAMERR_LIT ": 18");
    static_assert (fmt_error::parameter (19, result), UFO_PARAMERR_LIT ": 19");
    static_assert (fmt_error::parameter (20, result), UFO_PARAMERR_LIT ": 20");
    static_assert (fmt_error::parameter (21, result), UFO_PARAMERR_LIT ": 21");
    static_assert (fmt_error::parameter (22, result), UFO_PARAMERR_LIT ": 22");
    static_assert (fmt_error::parameter (23, result), UFO_PARAMERR_LIT ": 23");
    static_assert (fmt_error::parameter (24, result), UFO_PARAMERR_LIT ": 24");
    static_assert (fmt_error::parameter (25, result), UFO_PARAMERR_LIT ": 25");
    static_assert (fmt_error::parameter (26, result), UFO_PARAMERR_LIT ": 26");
    static_assert (fmt_error::parameter (27, result), UFO_PARAMERR_LIT ": 27");
    static_assert (fmt_error::parameter (28, result), UFO_PARAMERR_LIT ": 28");
    static_assert (fmt_error::parameter (29, result), UFO_PARAMERR_LIT ": 29");
    static_assert (fmt_error::parameter (30, result), UFO_PARAMERR_LIT ": 30");
    static_assert (fmt_error::parameter (31, result), UFO_PARAMERR_LIT ": 31");
    static_assert (fmt_error::parameter (32, result), UFO_PARAMERR_LIT ": 32");

    static_assert (fmt_error::placeholder (1,  result), UFO_PCHERR_LIT ": 1");
    static_assert (fmt_error::placeholder (2,  result), UFO_PCHERR_LIT ": 2");
    static_assert (fmt_error::placeholder (3,  result), UFO_PCHERR_LIT ": 3");
    static_assert (fmt_error::placeholder (4,  result), UFO_PCHERR_LIT ": 4");
    static_assert (fmt_error::placeholder (5,  result), UFO_PCHERR_LIT ": 5");
    static_assert (fmt_error::placeholder (6,  result), UFO_PCHERR_LIT ": 6");
    static_assert (fmt_error::placeholder (7,  result), UFO_PCHERR_LIT ": 7");
    static_assert (fmt_error::placeholder (8,  result), UFO_PCHERR_LIT ": 8");
    static_assert (fmt_error::placeholder (9,  result), UFO_PCHERR_LIT ": 9");
    static_assert (fmt_error::placeholder (10, result), UFO_PCHERR_LIT ": 10");
    static_assert (fmt_error::placeholder (11, result), UFO_PCHERR_LIT ": 11");
    static_assert (fmt_error::placeholder (12, result), UFO_PCHERR_LIT ": 12");
    static_assert (fmt_error::placeholder (13, result), UFO_PCHERR_LIT ": 13");
    static_assert (fmt_error::placeholder (14, result), UFO_PCHERR_LIT ": 14");
    static_assert (fmt_error::placeholder (15, result), UFO_PCHERR_LIT ": 15");
    static_assert (fmt_error::placeholder (16, result), UFO_PCHERR_LIT ": 16");
    static_assert (fmt_error::placeholder (17, result), UFO_PCHERR_LIT ": 17");
    static_assert (fmt_error::placeholder (18, result), UFO_PCHERR_LIT ": 18");
    static_assert (fmt_error::placeholder (19, result), UFO_PCHERR_LIT ": 19");
    static_assert (fmt_error::placeholder (20, result), UFO_PCHERR_LIT ": 20");
    static_assert (fmt_error::placeholder (21, result), UFO_PCHERR_LIT ": 21");
    static_assert (fmt_error::placeholder (22, result), UFO_PCHERR_LIT ": 22");
    static_assert (fmt_error::placeholder (23, result), UFO_PCHERR_LIT ": 23");
    static_assert (fmt_error::placeholder (24, result), UFO_PCHERR_LIT ": 24");
    static_assert (fmt_error::placeholder (25, result), UFO_PCHERR_LIT ": 25");
    static_assert (fmt_error::placeholder (26, result), UFO_PCHERR_LIT ": 26");
    static_assert (fmt_error::placeholder (27, result), UFO_PCHERR_LIT ": 27");
    static_assert (fmt_error::placeholder (28, result), UFO_PCHERR_LIT ": 28");
    static_assert (fmt_error::placeholder (29, result), UFO_PCHERR_LIT ": 29");
    static_assert (fmt_error::placeholder (30, result), UFO_PCHERR_LIT ": 30");
    static_assert (fmt_error::placeholder (31, result), UFO_PCHERR_LIT ": 31");
    static_assert (fmt_error::placeholder (32, result), UFO_PCHERR_LIT ": 32");

    static_assert (fmt_error::modifier (1,  result), UFO_MODIFERR_LIT " 1");
    static_assert (fmt_error::modifier (2,  result), UFO_MODIFERR_LIT " 2");
    static_assert (fmt_error::modifier (3,  result), UFO_MODIFERR_LIT " 3");
    static_assert (fmt_error::modifier (4,  result), UFO_MODIFERR_LIT " 4");
    static_assert (fmt_error::modifier (5,  result), UFO_MODIFERR_LIT " 5");
    static_assert (fmt_error::modifier (6,  result), UFO_MODIFERR_LIT " 6");
    static_assert (fmt_error::modifier (7,  result), UFO_MODIFERR_LIT " 7");
    static_assert (fmt_error::modifier (8,  result), UFO_MODIFERR_LIT " 8");
    static_assert (fmt_error::modifier (9,  result), UFO_MODIFERR_LIT " 9");
    static_assert (fmt_error::modifier (10, result), UFO_MODIFERR_LIT " 10");
    static_assert (fmt_error::modifier (11, result), UFO_MODIFERR_LIT " 11");
    static_assert (fmt_error::modifier (12, result), UFO_MODIFERR_LIT " 12");
    static_assert (fmt_error::modifier (13, result), UFO_MODIFERR_LIT " 13");
    static_assert (fmt_error::modifier (14, result), UFO_MODIFERR_LIT " 14");
    static_assert (fmt_error::modifier (15, result), UFO_MODIFERR_LIT " 15");
    static_assert (fmt_error::modifier (16, result), UFO_MODIFERR_LIT " 16");
    static_assert (fmt_error::modifier (17, result), UFO_MODIFERR_LIT " 17");
    static_assert (fmt_error::modifier (18, result), UFO_MODIFERR_LIT " 18");
    static_assert (fmt_error::modifier (19, result), UFO_MODIFERR_LIT " 19");
    static_assert (fmt_error::modifier (20, result), UFO_MODIFERR_LIT " 20");
    static_assert (fmt_error::modifier (21, result), UFO_MODIFERR_LIT " 21");
    static_assert (fmt_error::modifier (22, result), UFO_MODIFERR_LIT " 22");
    static_assert (fmt_error::modifier (23, result), UFO_MODIFERR_LIT " 23");
    static_assert (fmt_error::modifier (24, result), UFO_MODIFERR_LIT " 24");
    static_assert (fmt_error::modifier (25, result), UFO_MODIFERR_LIT " 25");
    static_assert (fmt_error::modifier (26, result), UFO_MODIFERR_LIT " 26");
    static_assert (fmt_error::modifier (27, result), UFO_MODIFERR_LIT " 27");
    static_assert (fmt_error::modifier (28, result), UFO_MODIFERR_LIT " 28");
    static_assert (fmt_error::modifier (29, result), UFO_MODIFERR_LIT " 29");
    static_assert (fmt_error::modifier (30, result), UFO_MODIFERR_LIT " 30");
    static_assert (fmt_error::modifier (31, result), UFO_MODIFERR_LIT " 31");
    static_assert (fmt_error::modifier (32, result), UFO_MODIFERR_LIT " 32");

    return true;
}
//------------------------------------------------------------------------------
#undef UFO_PARAMERR_LIT
#undef UFO_PCHERR_LIT
#undef UFO_MODIFERR_LIT
//------------------------------------------------------------------------------
} //ufo

#endif /* UFO_LOG_FORMAT_VALIDATOR_HPP_ */

