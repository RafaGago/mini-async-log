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

#ifndef UFO_LOG_NON_INTEGRAL_HPP_
#define UFO_LOG_NON_INTEGRAL_HPP_

#include <type_traits>
#include <cassert>
#include <ufo_log/util/system.hpp>
#include <ufo_log/serialization/fields.hpp>
#include <ufo_log/util/integer.hpp>
#include <ufo_log/serialization/basic_encoder_decoder.hpp>

namespace ufo { namespace ser {

//------------------------------------------------------------------------------
template <class T, class ret>
struct enable_float_double_bool :
        public std::enable_if<
                std::is_floating_point<T>::value || std::is_same<T, bool>::value,
                ret
                >
{};
//------------------------------------------------------------------------------
class non_integral : public basic_encoder_decoder                               //a very relaxed and practical definition, float, double and bool.
{
public:
    typedef non_integral_field field;
    //--------------------------------------------------------------------------
    static uword bytes_required (bool)
    {
        return 0 + sizeof (field);
    }
    //--------------------------------------------------------------------------
    static uword bytes_required (float)
    {
        return sizeof (float) + sizeof (field);
    }
    //--------------------------------------------------------------------------
    static uword bytes_required (double)
    {
        return sizeof (double) + sizeof (field);
    }
    //--------------------------------------------------------------------------
    static field get_field (bool val, uword bytes_required)
    {
        field f;
        f.fclass   = ufo_numeric;
        f.nclass   = ufo_non_integral;
        f.niclass  = ufo_bool;
        f.bool_val = val ? 1 : 0;
        return f;
    }
    //--------------------------------------------------------------------------
    static field get_field (float, uword bytes_required)
    {
        field f;
        f.fclass   = ufo_numeric;
        f.nclass   = ufo_non_integral;
        f.niclass  = ufo_float;
        f.bool_val = 0;
        return f;
    }
    //--------------------------------------------------------------------------
    static field get_field (double, uword bytes_required)
    {
        field f;
        f.fclass   = ufo_numeric;
        f.nclass   = ufo_non_integral;
        f.niclass  = ufo_double;
        f.bool_val = 0;
        return f;
    }
    //--------------------------------------------------------------------------
    static u8* encode (u8* ptr, u8* end, bool, field f)
    {
        ptr = encode_type (ptr, end, f);
        return ptr;
    }
    //--------------------------------------------------------------------------
    static u8* encode (u8* ptr, u8* end, float v, field f)
    {
        ptr = encode_type (ptr, end, f);
        ptr = encode_type (ptr, end, v);
        return ptr;
    }
    //--------------------------------------------------------------------------
    static u8* encode (u8* ptr, u8* end, double v, field f)
    {
        ptr = encode_type (ptr, end, f);
        ptr = encode_type (ptr, end, v);
        return ptr;
    }
    //--------------------------------------------------------------------------
    static const u8* decode (bool& v, field f, const u8* ptr, const u8* end)
    {
        v = f.bool_val;
        return ptr;
    }
    //--------------------------------------------------------------------------
    static const u8* decode (float& v, field f, const u8* ptr, const u8* end)
    {
        return decode_type (v, ptr, end);
    }
    //--------------------------------------------------------------------------
    static const u8* decode (double& v, field f, const u8* ptr, const u8* end)
    {
        return decode_type (v, ptr, end);
    }
    //--------------------------------------------------------------------------
}; //class integral
//------------------------------------------------------------------------------

}} //namespaces
#endif /* UFO_LOG_NON_INTEGRAL_HPP_ */
