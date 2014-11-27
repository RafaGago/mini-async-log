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

#ifndef UFO_LOG_HEADER_HPP_
#define UFO_LOG_HEADER_HPP_

#include <ufo_log/util/system.hpp>
#include <ufo_log/util/integer.hpp>
#include <ufo_log/frontend_types.hpp>
#include <ufo_log/serialization/fields.hpp>
#include <ufo_log/serialization/integral.hpp>
#include <ufo_log/serialization/header_data.hpp>

namespace ufo { namespace ser {

//------------------------------------------------------------------------------
class header : public encoder_decoder_base
{
public:
    //--------------------------------------------------------------------------
    typedef header_field field;
    //--------------------------------------------------------------------------
    static uword bytes_required (header_data v)
    {
        uword sz;

        return v.has_tstamp ? (integral::raw_bytes_required (v.tstamp)) : 0 +
#ifndef UFO_COMPILE_TIME_FMT_CHECK
                integral::raw_bytes_required (v.msg_bytes) +
#endif
                sizeof (const char*) +
                sizeof (field);
    }
    //--------------------------------------------------------------------------
    static field get_field (header_data v, uword bytes_required)
    {
        field f;
        f.arity           = v.arity;
        f.severity        = v.severity;
        f.no_timestamp    = v.has_tstamp ? 0 : 1;
        f.timestamp_bytes = v.has_tstamp ?
                                integral::raw_bytes_required (v.tstamp) - 1 :
                                0;
#ifndef UFO_COMPILE_TIME_FMT_CHECK
        f.length_bytes    = integral::raw_bytes_required (v.msg_bytes) - 1;
#endif
        return f;
    }
    //--------------------------------------------------------------------------
    static u8* encode (u8* ptr, u8* end, field f, header_data hd)
    {
        ptr = encode_type (ptr, end, f);
        ptr = encode_type (ptr, end, hd.fmt);
        if (hd.has_tstamp)
        {
            ptr = integral::raw_encode_unsigned(
                    ptr, end, ((uword) f.timestamp_bytes) + 1, hd.tstamp
                    );
        }
#ifndef UFO_COMPILE_TIME_FMT_CHECK
        ptr = integral::raw_encode_unsigned(
                ptr, end, ((uword) f.length_bytes) + 1, hd.msg_bytes
                );
#endif
        return ptr;
    }
    //--------------------------------------------------------------------------
    static const u8* decode (header_data& hd, const u8* ptr, const u8* end)
    {
        field f;
        ptr = decode_type (f, ptr, end);
        ptr = decode_type (hd.fmt, ptr, end);

        hd.arity      = f.arity;
        hd.has_tstamp = f.no_timestamp ? 0 : 1;
        hd.tstamp     = 0;
        hd.severity   = f.severity;

        if (hd.has_tstamp)
        {
            ptr = integral::raw_decode_unsigned(
                    hd.tstamp, ((uword) f.timestamp_bytes) + 1 , ptr, end
                    );
        }
#ifndef UFO_COMPILE_TIME_FMT_CHECK
        ptr = integral::raw_decode_unsigned(
                hd.msg_bytes, ((uword) f.length_bytes) + 1, ptr, end
                );
#endif
        return ptr;
    }
    //--------------------------------------------------------------------------
}; //class integral
//------------------------------------------------------------------------------

}} //namespaces

#endif /* UFO_LOG_HEADER_HPP_ */
