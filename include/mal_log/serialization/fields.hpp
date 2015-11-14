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

#ifndef MAL_LOG_FIELDS_HPP_
#define MAL_LOG_FIELDS_HPP_

#include <mal_log/util/integer.hpp>
#include <mal_log/util/system.hpp>

namespace mal { namespace ser {
//------------------------------------------------------------------------------
static const uword numeric_bytes_bits = 3;
//------------------------------------------------------------------------------
struct header_field {
    typedef u16 raw_type;

    static const uword arity_bits           = 6;                                //0 = just fmt string
    static const uword severity_bits        = 3;
    static const uword no_timestamp_bits    = 1;
    static const uword is_sync_bits         = 1;

    raw_type arity           : arity_bits;
    raw_type severity        : severity_bits;
    raw_type timestamp_bytes : numeric_bytes_bits;                              //1 to 8
    raw_type no_timestamp    : no_timestamp_bits;
    raw_type is_sync         : is_sync_bits;
};
//------------------------------------------------------------------------------
static_assert (sizeof (header_field) == sizeof (header_field::raw_type), "");
//------------------------------------------------------------------------------
enum field_class {
    mal_numeric,
    mal_non_numeric,
};
//------------------------------------------------------------------------------
static const uword field_class_bits = 1;
//------------------------------------------------------------------------------
enum numeric_class {
    mal_integral,
    mal_non_integral,
};
//------------------------------------------------------------------------------
static const uword numeric_class_bits = 1;
//------------------------------------------------------------------------------
enum non_integral_class {
    mal_double,
    mal_float,
    mal_bool,
};
//------------------------------------------------------------------------------
static const uword non_integral_class_bits = 2;
//------------------------------------------------------------------------------
enum non_numeric_class {
    mal_c_str,
    mal_ptr,
    mal_deep_copied_str,
    mal_deep_copied_mem,
//    shared_ptr_str,
//    shared_ptr_mem,
};
//------------------------------------------------------------------------------
static const uword non_numeric_class_bits = 3;
//------------------------------------------------------------------------------
struct integral_field {
    typedef u8 raw_type;

    enum original_type_values {
        b8, b16, b32, b64
    };
    static const uword original_type_bits = 2;
    static const uword is_negative_bits   = 1;

    raw_type fclass        : field_class_bits;
    raw_type nclass        : numeric_class_bits;
    raw_type bytes         : numeric_bytes_bits;
    raw_type original_type : original_type_bits;
    raw_type is_negative   : is_negative_bits;
};
//------------------------------------------------------------------------------
static_assert(
        sizeof (integral_field) == sizeof (integral_field::raw_type), ""
        );
//------------------------------------------------------------------------------
struct non_integral_field {
    typedef u8 raw_type;
    static const uword bool_val_bits = 1;

    raw_type fclass    : field_class_bits;
    raw_type nclass    : numeric_class_bits;
    raw_type niclass   : non_integral_class_bits;
    raw_type bool_val  : bool_val_bits;
};
//------------------------------------------------------------------------------
static_assert(
        sizeof (non_integral_field) == sizeof (non_integral_field::raw_type), ""
        );
//------------------------------------------------------------------------------
struct non_numeric_field {
    typedef u8 raw_type;
    static const uword nnclass_bits = 2;

    raw_type fclass                   : field_class_bits;
    raw_type nnclass                  : nnclass_bits;
    raw_type deep_copied_length_bytes : numeric_bytes_bits;
};
//------------------------------------------------------------------------------
static_assert(
        sizeof (non_numeric_field) == sizeof (non_numeric_field::raw_type), ""
        );
//------------------------------------------------------------------------------
struct generic_decoder_field
{
    typedef u8 raw_type;

    raw_type fclass : field_class_bits;
    raw_type nclass : numeric_class_bits;
};
//------------------------------------------------------------------------------
static_assert(
        sizeof (generic_decoder_field) ==
                sizeof (generic_decoder_field::raw_type),
        ""
        );
//------------------------------------------------------------------------------
union decoding_field {
    typedef u8 raw_type;

    generic_decoder_field gen;
    integral_field        num_int;
    non_integral_field    nom_no_int;
    non_numeric_field     no_num;
};
//------------------------------------------------------------------------------
static_assert(
        sizeof (decoding_field) == sizeof (decoding_field::raw_type), ""
        );
//------------------------------------------------------------------------------
}} //namespaces

#endif /* MAL_LOG_FIELDS_HPP_ */
