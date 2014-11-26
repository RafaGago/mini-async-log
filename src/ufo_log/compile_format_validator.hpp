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
        return (f == fmt::hex)               ? true :
               (f == fmt::full_width_spaces) ? true :
               (f == fmt::full_width_zeroes) ? true :
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
        return (f == fmt::full_width_spaces) ? true :
               (f == fmt::full_width_zeroes) ? true :
                            false;
    }
    //--------------------------------------------------------------------------
    template <class T>
    static constexpr bool is_formatting_valid (char c, ...)
    {
        return false;
    }
    //--------------------------------------------------------------------------
#if NEW_FORMATS_UNDER_DEVELOPMENT
    //--------------------------------------------------------------------------
    static constexpr bool token_with_fmt (literal l, uword i)
    {
        return (i <= (l.size() - 3)) &&
               (l[i]     == fmt::placeholder_open) &&
               (l[i + 1] == fmt::placeholder_close);
    }
#else
    //--------------------------------------------------------------------------
    static constexpr bool token_with_fmt (literal l, uword i)
    {
        return false;
    }
#endif
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
    static constexpr word consume_param_impl (literal l, uword i, word arity)
    {
        return scan<args...> (l, i + 2, arity + 1);
    }
    //--------------------------------------------------------------------------
    template <class dummy>
    static constexpr word consume_param_impl (literal l, uword i, word arity)
    {
        return -(arity + 1);
    }
    //--------------------------------------------------------------------------
    template <class... args>
    static constexpr word consume_param (literal l, uword i, word arity)
    {
        return consume_param_impl<at_least_one<>, args...> (l, i, arity);
    }
    //--------------------------------------------------------------------------
    template <class dummy, class T, class... args>
    static constexpr word consume_param_with_fmt_impl(
                            literal l, uword i, word arity
                            )
    {
        typedef typename fmt_check_adapt<T>::type type;
        return is_formatting_valid<type> (l[i + 1], (type*) nullptr) ?
                scan<args...> (l, i + 3, arity + 1) :
                -(arity + 1);
    }
    //--------------------------------------------------------------------------
    template <class dummy>
    static constexpr word consume_param_with_fmt_impl(
                            literal l, uword i, word arity
                            )
    {
        return -(arity + 1);
    }
    //--------------------------------------------------------------------------
    template <class... args>
    static constexpr word consume_param_with_fmt(
                            literal l, uword i, word arity
                            )
    {
        return consume_param_with_fmt_impl<
                                at_least_one<>, args...
                                >(l, i, arity);
    }
    //--------------------------------------------------------------------------
    template <class... args>
    static constexpr word check_arity_match (word arity)
    {
        return (sizeof... (args) == 0) ? arity : -(arity + 1);
    }
    //--------------------------------------------------------------------------
    template <class... args>
    static constexpr word scan (literal l, uword i, word arity)
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
template <word result>
constexpr bool trigger_format_error()
{
#define error_literal "invalid format string or arity mismatch"
#define error_literal_ext error_literal " in parameter "

    static_assert (result != -1,  error_literal_ext "1");
    static_assert (result != -2,  error_literal_ext "2");
    static_assert (result != -3,  error_literal_ext "3");
    static_assert (result != -4,  error_literal_ext "4");
    static_assert (result != -5,  error_literal_ext "5");
    static_assert (result != -6,  error_literal_ext "6");
    static_assert (result != -7,  error_literal_ext "7");
    static_assert (result != -8,  error_literal_ext "8");
    static_assert (result != -9,  error_literal_ext "9");
    static_assert (result != -10, error_literal_ext "10");
    static_assert (result != -11, error_literal_ext "11");
    static_assert (result != -12, error_literal_ext "12");
    static_assert (result != -13, error_literal_ext "13");
    static_assert (result != -14, error_literal_ext "14");
    static_assert (result != -15, error_literal_ext "15");
    static_assert (result != -16, error_literal_ext "16");
    static_assert (result != -17, error_literal_ext "17");
    static_assert (result != -18, error_literal_ext "18");
    static_assert (result != -19, error_literal_ext "19");
    static_assert (result != -20, error_literal_ext "20");
    static_assert (result != -21, error_literal_ext "21");
    static_assert (result != -22, error_literal_ext "22");
    static_assert (result != -23, error_literal_ext "23");
    static_assert (result != -24, error_literal_ext "24");
    static_assert (result != -25, error_literal_ext "25");
    static_assert (result != -26, error_literal_ext "26");
    static_assert (result != -27, error_literal_ext "27");
    static_assert (result != -28, error_literal_ext "28");
    static_assert (result != -29, error_literal_ext "29");
    static_assert (result != -30, error_literal_ext "30");
    static_assert (result != -31, error_literal_ext "31");
    static_assert (result != -32, error_literal_ext "32");
    static_assert (result >= -32, error_literal);

    return true;

#undef error_literal
#undef error_literal_ext
}
//------------------------------------------------------------------------------

} //ufo

#endif /* UFO_LOG_FORMAT_VALIDATOR_HPP_ */

