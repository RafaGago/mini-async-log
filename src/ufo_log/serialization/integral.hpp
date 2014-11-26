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
#include <ufo_log/serialization/fields.hpp>
#include <ufo_log/serialization/encoder_decoder_base.hpp>
#include <type_traits>

namespace ufo { namespace ser {

//------------------------------------------------------------------------------
class integral : public encoder_decoder_base
{
private:
    //--------------------------------------------------------------------------
    template <class ret, class T>
    struct enable_signed
    {
        typedef typename std::enable_if<
                std::is_signed<T>::type && !std::is_floating_point<T>::type,
                ret
                >::type type;
    };
    //--------------------------------------------------------------------------
    template <class ret, class T>
    struct enable_unsigned
    {
        typedef typename std::enable_if<
                    std::is_unsigned<T>::type && !std::is_same<T, bool>::type,
                    uword
                    >::type type;
    };
public:
    //--------------------------------------------------------------------------
    typedef integral_field field;
    //--------------------------------------------------------------------------
    template <class T>
    typename enable_unsigned<uword, T>::type
    static required_bytes (T val)
    {
        return highest_used_byte (val) + 1 + sizeof (field);
    }
    //--------------------------------------------------------------------------
    template <class T>
    typename enable_signed<uword, T>::type
    static required_bytes (T val)
    {
        val    = (val >= 0) ? val : prepare_negative (val);
        return highest_used_byte (val) + 1 + sizeof (field);
    }
    //--------------------------------------------------------------------------
    static uword required_bytes (i8 val) { return 1 + sizeof (field); }
    //--------------------------------------------------------------------------
    template <class T>
    typename enable_unsigned<field, T>::type
    get_field (T val, uword required_bytes)
    {
        return get_field_impl (val, required_bytes, false);
    }
    //--------------------------------------------------------------------------
    template <class T>
    typename enable_signed<field, T>::type
    get_field (T val, uword required_bytes)
    {
        return get_field_impl (val, required_bytes, val < 0);
    }
    //--------------------------------------------------------------------------
    template <class T>
    typename enable_unsigned<void, T>::type
    encode (u8* ptr, u8* end, field f, T val)
    {
        return encode_impl (ptr, end, f, val);
    }
    //--------------------------------------------------------------------------
    template <class T>
    typename enable_signed<void, T>::type
    encode (u8* ptr, u8* end, field f, T val)
    {
        return encode_impl(
                ptr, end, f, (f.is_negative) ? val : prepare_negative (val)
                );
    }
    //--------------------------------------------------------------------------
    template <class T>
    typename enable_unsigned<void, T>::type
    decode (T& val, field f, const u8* ptr)
    {
        decode_unsigned_impl (val, f, ptr);
    }
    //--------------------------------------------------------------------------
    template <class T>
    typename enable_signed<void, T>::type
    decode (T& val, field f, const u8* ptr)
    {
        decode_unsigned_impl (val, f, ptr);
        val = !f.is_negative ? val : reconstruct_negative (val);
    }
    //--------------------------------------------------------------------------
private:
    //--------------------------------------------------------------------------
    template <class T>
    void decode_unsigned_impl (T& val, field f, const u8* ptr)
    {
        val = 0;
        for (uword i = 0; i < (f.bytes + 1); ++i, ++ptr)
        {
            val |= ((T) *ptr) << (i * 8);
        }
    }
    //--------------------------------------------------------------------------
    template <class T>
    void encode_impl (u8* ptr, u8* end, field f, T val)
    {
        uword to_encode = f.bytes + 1;

        assert (ptr && end);
        assert ((ptr + (to_encode+ sizeof f)) < end);
        assert (to_encode <= sizeof (T));

        ptr = encode_type (ptr, end, f);

        for (uword i = 0; i < to_encode; ++i, ++ptr)
        {
            *ptr = (u8) (val >> (i * 8));
        }
    }
    //--------------------------------------------------------------------------
    template <class T>
    field get_field_impl (T val, uword required_bytes, bool negative)
    {
        static_assert(
            sizeof (T) <= 8,
            "f.original type needs a proper compile time log2"
            );
        assert (required_bytes <= sizeof (T));
        field f;
        f.fclass        = ufo_numeric;
        f.nclass        = ufo_integral;
        f.bytes         = (required_bytes - 1 - sizeof f);
        f.original_type = ((sizeof val / 2) == 4) ? 3 : (sizeof val / 2);
        f.is_negative   = negative;
    }
    //--------------------------------------------------------------------------
    template <class T>
    T prepare_negative (T val)
    {
        static const T sign_mask = ~(1 << ((sizeof val * 8) - 1));
        return ~(val & sign_mask);
    }
    //--------------------------------------------------------------------------
    template <class T>
    T reconstruct_negative (T val)
    {
        static const T sign_mask = (1 << ((sizeof val * 8) - 1));
        return (~val) | sign_mask;
    }
    //--------------------------------------------------------------------------
}; //class integral
//------------------------------------------------------------------------------

}} //namespaces

#endif /* UFO_LOG_INTEGRAL_HPP_ */
