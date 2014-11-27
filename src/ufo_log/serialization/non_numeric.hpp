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

#ifndef UFO_LOG_NON_NUMERIC_HPP_
#define UFO_LOG_NON_NUMERIC_HPP_

#include <ufo_log/util/system.hpp>
#include <ufo_log/util/integer.hpp>
#include <ufo_log/frontend_types.hpp>
#include <ufo_log/serialization/fields.hpp>
#include <ufo_log/serialization/integral.hpp>
#include <cstring>
#include <cassert>

namespace ufo { namespace ser {
//------------------------------------------------------------------------------
class non_numeric : public encoder_decoder_base
{
public:
    //--------------------------------------------------------------------------
    typedef non_numeric_field field;
    //--------------------------------------------------------------------------
    static UFO_CONSTEXPR uword bytes_required (const char* str)
    {
        return sizeof (field) + sizeof (str);
    }
    //--------------------------------------------------------------------------
    static uword bytes_required (deep_copy_bytes b)
    {
        return bytes_required ((delimited_mem) b);
    }
    //--------------------------------------------------------------------------
    static uword bytes_required (deep_copy_string s)
    {
        return bytes_required ((delimited_mem) s);
    }
    //--------------------------------------------------------------------------
    static field get_field (deep_copy_bytes b, uword bytes_required)
    {
        assert (bytes_required > sizeof (field) + b.size);
        field f;
        f.fclass                   = ufo_non_numeric;
        f.nnclass                  = ufo_deep_copied_mem;
        f.deep_copied_length_bytes = bytes_required - b.size - sizeof (f) - 1;
        return f;
    }
    //--------------------------------------------------------------------------
    static field get_field (deep_copy_string s, uword bytes_required)
    {
        assert (bytes_required > sizeof (field) + s.size);
        field f;
        f.fclass                   = ufo_non_numeric;
        f.nnclass                  = ufo_deep_copied_str;
        f.deep_copied_length_bytes = bytes_required - s.size - sizeof (f) - 1;
        return f;
    }
    //--------------------------------------------------------------------------
    static field get_field (const char*, uword bytes_required)
    {
        field f;
        f.fclass                    = ufo_non_numeric;
        f.nnclass                   = ufo_c_str;
        f.deep_copied_length_bytes  = 0;
        return f;
    }
    //--------------------------------------------------------------------------
    static u8* encode (u8* ptr, u8* end, field f, const char* str)
    {
        ptr = encode_type (ptr, end, f);
        ptr = encode_type (ptr, end, str);
        return ptr;
    }
    //--------------------------------------------------------------------------
    static u8* encode (u8* ptr, u8* end, field f, deep_copy_bytes b)
    {
        return encode (ptr, end, f, (delimited_mem) b);
    }
    //--------------------------------------------------------------------------
    static u8* encode (u8* ptr, u8* end, field f, deep_copy_string s)
    {
        return encode (ptr, end, f, (delimited_mem) s);
    }
    //--------------------------------------------------------------------------
    static u8* decode (const char*& str, field, const u8* ptr, const u8* end)
    {
        ptr = decode_type (str, ptr, end);
        return ptr;
    }
    //--------------------------------------------------------------------------
    static u8* decode(
                deep_copy_bytes& b, field f, const u8* ptr, const u8* end
                )
    {
        return decode ((delimited_mem&) b, f, ptr, end);
    }
    //--------------------------------------------------------------------------
    static u8* decode(
                deep_copy_string& s, field f, const u8* ptr, const u8* end
                )
    {
        return decode ((delimited_mem&) s, f, ptr, end);
    }
    //--------------------------------------------------------------------------
private:
    //--------------------------------------------------------------------------
    static uword bytes_required (delimited_mem m)
    {
        return sizeof (field) +
                integral::raw_bytes_required (m.size) +
                m.size
                ;
    }
    //--------------------------------------------------------------------------
    static u8* encode (u8* ptr, u8* end, field f, delimited_mem m)
    {
        ptr = encode_type (ptr, end, f);
        ptr = integral::raw_encode_unsigned(
                ptr, end, m.size, ((uword) f.deep_copied_length_bytes) + 1
                );
        assert (ptr + m.size <= end);
        std::memcpy (ptr, m.mem, m.size);
        return ptr + m.size;
    }
    //--------------------------------------------------------------------------
    static const u8* decode(
                        delimited_mem& m, field f, const u8* ptr, const u8* end
                        )
    {
        ptr   = integral::raw_decode_unsigned(
                m.size, ((uword) f.deep_copied_length_bytes) + 1, ptr, end
                );
        m.mem = ptr;
        assert (ptr + m.size <= end);
        return ptr + m.size;
    }
    //--------------------------------------------------------------------------
}; //class non_numeric
//------------------------------------------------------------------------------

}} //namespaces

#endif /* UFO_LOG_NON_NUMERIC_HPP_ */
