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

#ifndef UFO_LOG_INTEGRAL_HPP_
#define UFO_LOG_INTEGRAL_HPP_

#include <cassert>
#include <ufo_log/util/system.hpp>
#include <ufo_log/util/integer.hpp>
#include <ufo_log/util/integer_bits.hpp>
#include <ufo_log/util/integral_enable_if.hpp>
#include <ufo_log/serialization/fields.hpp>
#include <ufo_log/serialization/basic_encoder_decoder.hpp>

namespace ufo { namespace ser {

//------------------------------------------------------------------------------
class integral : public basic_encoder_decoder
{
public:
    //--------------------------------------------------------------------------
    typedef integral_field field;
    //--------------------------------------------------------------------------
    template <class T>
    static typename enable_if_unsigned<T, uword>::type
    bytes_required (T val)                                               //todo: make constextr...
    {
        return raw_bytes_required (val) + sizeof (field);
    }
    //--------------------------------------------------------------------------
    template <class T>
    static typename enable_if_signed<T, uword>::type
    bytes_required (T val)                                               //todo make constexpr
    {
        val    = (val >= 0) ? val : prepare_negative (val);
        typename std::make_unsigned<T>::type v = val;
        return raw_bytes_required (v) + sizeof (field);
    }
    //--------------------------------------------------------------------------
    static uword bytes_required (i8 val)
    {
        return 1 + sizeof (field);
    }
    //--------------------------------------------------------------------------
    template <class T>
    static typename enable_if_unsigned<T, field>::type
    get_field (T val, uword bytes_required)
    {
        return get_field_impl (val, bytes_required, false);
    }
    //--------------------------------------------------------------------------
    template <class T>
    static typename enable_if_signed<T, field>::type
    get_field (T val, uword bytes_required)
    {
        return get_field_impl (val, bytes_required, val < 0);
    }
    //--------------------------------------------------------------------------
    template <class T>
    static typename enable_if_unsigned<T, u8*>::type
    encode (u8* ptr, u8* end, field f, T val)
    {
        return encode_impl (ptr, end, f, val);
    }
    //--------------------------------------------------------------------------
    template <class T>
    static typename enable_if_signed<T, u8*>::type
    encode (u8* ptr, u8* end, field f, T val)
    {
        return encode_impl(
                ptr, end, f, (f.is_negative) ? val : prepare_negative (val)
                );
    }
    //--------------------------------------------------------------------------
    template <class T>
    static typename enable_if_unsigned<T, const u8*>::type
    decode (T& val, field f, const u8* ptr, const u8* end)
    {
        return raw_decode_unsigned (val, ((uword) f.bytes) + 1, ptr, end);
    }
    //--------------------------------------------------------------------------
    template <class T>
    static typename enable_if_signed<T, const u8*>::type
    decode (T& val, field f, const u8* ptr, const u8* end)
    {
        ptr = decode_unsigned (val, ((uword) f.bytes) + 1, ptr, end);
        val = !f.is_negative ? val : reconstruct_negative (val);
        return ptr;
    }
    //--------------------------------------------------------------------------
    template <class T>
    static typename enable_if_unsigned<T, uword>::type
    raw_bytes_required (T val)
    {
#ifndef UFO_NO_VARIABLE_INTEGER_WIDTH
        return highest_used_byte (val) + 1;
#else
        return sizeof (T);
#endif
    }
    //--------------------------------------------------------------------------
    template <class T>
    static typename enable_if_unsigned<T, u8*>::type
    raw_encode_unsigned (u8* ptr, u8* end, uword size, T val)
    {
        return encode_unsigned (ptr, end, size, val);
    }
    //--------------------------------------------------------------------------
    template <class T>
    static typename enable_if_unsigned<T, const u8*>::type
    raw_decode_unsigned (T& val, uword size, const u8* ptr, const u8* end)
    {
        return decode_unsigned (val, size, ptr, end);
    }
    //--------------------------------------------------------------------------
private:
    //--------------------------------------------------------------------------
    template <class T>
    static const u8* decode_unsigned(
            T& val, uword size, const u8* ptr, const u8* end
            )
    {
        assert (ptr + size <= end);
        val = 0;
        for (uword i = 0; i < size; ++i, ++ptr)
        {
            val |= ((T) *ptr) << (i * 8);
        }
        return ptr;
    }
    //--------------------------------------------------------------------------
    template <class T>
    static u8* encode_unsigned(
            u8* ptr, u8* end, uword size, T val
            )
    {
        assert (ptr && end);
        assert (ptr + size <= end);
        assert (size <= sizeof (T));

        for (uword i = 0; i < size; ++i, ++ptr)
        {
            *ptr = (u8) (val >> (i * 8));
        }
        return ptr;
    }
    //--------------------------------------------------------------------------
    template <class T>
    static u8* encode_impl (u8* ptr, u8* end, field f, T val)
    {
        ptr = encode_type (ptr, end, f);
        ptr = encode_unsigned (ptr, end, ((uword) f.bytes) + 1, val);
        return ptr;
    }
    //--------------------------------------------------------------------------
    template <class T>
    static field get_field_impl (T val, uword bytes_required, bool negative)
    {
        static_assert(
            sizeof (T) <= 8,
            "f.original type needs a proper compile time log2"
            );
        assert (bytes_required <= (sizeof (T) + sizeof (field)));
        field f;
        f.fclass        = ufo_numeric;
        f.nclass        = ufo_integral;
        f.bytes         = (bytes_required - 1 - sizeof f);
        assert (f.bytes == (bytes_required - 1 - sizeof f));
        f.original_type = ((sizeof val / 2) == 4) ? 3 : (sizeof val / 2);
        f.is_negative   = negative;
        return f;
    }
    //--------------------------------------------------------------------------
    template <class T>
    static T prepare_negative (T val)
    {
        static const T sign_mask = ~(((T) 1) << ((sizeof val * 8) - 1));
        return ~(val & sign_mask);
    }
    //--------------------------------------------------------------------------
    template <class T>
    static T reconstruct_negative (T val)
    {
        static const T sign_mask = ((T) 1) << ((sizeof val * 8) - 1);
        return (~val) | sign_mask;
    }
    //--------------------------------------------------------------------------
}; //class integral
//------------------------------------------------------------------------------

}} //namespaces

#endif /* UFO_LOG_INTEGRAL_HPP_ */
