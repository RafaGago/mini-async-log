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

#include <cassert>
#include <ufo_log/util/system.hpp>
#include <ufo_log/serialization/fields.hpp>
#include <ufo_log/util/integer.hpp>
#include <ufo_log/serialization/encoder_decoder_base.hpp>

namespace ufo { namespace ser {

//------------------------------------------------------------------------------
class non_integral : public encoder_decoder_base
{
public:
    typedef non_integral_field field;
    //--------------------------------------------------------------------------
    static uword required_bytes (bool)
    {
        return 0 + sizeof (field);
    }
    //--------------------------------------------------------------------------
    static uword required_bytes (float)
    {
        return sizeof (float) + sizeof (field);
    }
    //--------------------------------------------------------------------------
    static uword required_bytes (double)
    {
        return sizeof (double) + sizeof (field);
    }
    //--------------------------------------------------------------------------
    field get_field (bool val, uword required_bytes)
    {
        field f;
        f.fclass   = ufo_numeric;
        f.nclass   = ufo_non_integral;
        f.niclass  = ufo_bool;
        f.bool_val = val ? 1 : 0;
        return f;
    }
    //--------------------------------------------------------------------------
    field get_field (float, uword required_bytes)
    {
        field f;
        f.fclass   = ufo_numeric;
        f.nclass   = ufo_non_integral;
        f.niclass  = ufo_float;
        f.bool_val = 0;
        return f;
    }
    //--------------------------------------------------------------------------
    field get_field (double, uword required_bytes)
    {
        field f;
        f.fclass   = ufo_numeric;
        f.nclass   = ufo_non_integral;
        f.niclass  = ufo_double;
        f.bool_val = 0;
        return f;
    }
    //--------------------------------------------------------------------------
    void encode (u8* ptr, u8* end, field f, bool)
    {
        ptr = encode_type (ptr, end, f);
    }
    //--------------------------------------------------------------------------
    void encode (u8* ptr, u8* end, field f, float v)
    {
        ptr = encode_type (ptr, end, f);
        encode_type (ptr, end, v);
    }
    //--------------------------------------------------------------------------
    void encode (u8* ptr, u8* end, field f, double v)
    {
        ptr = encode_type (ptr, end, f);
        encode_type (ptr, end, v);
    }
    //--------------------------------------------------------------------------
    void decode (bool& v, field f, const u8* ptr)
    {
        v = f.bool_val;
    }
    //--------------------------------------------------------------------------
    void decode (float& v, field f, const u8* ptr)
    {
        decode_type (v, ptr);
    }
    //--------------------------------------------------------------------------
    void decode (double& v, field f, const u8* ptr)
    {
        decode_type (v, ptr);
    }
    //--------------------------------------------------------------------------
}; //class integral
//------------------------------------------------------------------------------

#endif /* UFO_LOG_NON_INTEGRAL_HPP_ */
