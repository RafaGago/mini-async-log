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

#ifndef UFO_LOG_FIELDS_HPP_
#define UFO_LOG_FIELDS_HPP_

#include <util/integer.hpp>
#include <util/system.hpp>

namespace ufo { namespace ser {

static const uword arity_bits         = 5;
static const uword severity_bits      = 3;
static const uword numeric_bytes_bits = 3;

#ifdef UFO_COMPILE_TIME_FMT_CHECK

//------------------------------------------------------------------------------
struct header_field
{
    typedef u8 raw_type;

    static const uword severity_bits        = 3;
    static const uword timestamp_bytes_bits = 3;

    raw_type severity        : severity_bits;
    raw_type timestamp_bytes : numeric_bytes_bits;                              //1 to 8
};
//------------------------------------------------------------------------------

#else

//------------------------------------------------------------------------------
struct header_field
{
    typedef u16 raw_type;

    static const uword arity_bits           = 10;                               //it could work work with less bits, but 5 were the absolute minimum, the type needed to be a u16 anyways.
    static const uword severity_bits        = 3;
    static const uword timestamp_bytes_bits = 3;

    raw_type arity           : arity_bits;                                      //arity 0 = just fmt string
    raw_type severity        : severity_bits;
    raw_type timestamp_bytes : numeric_bytes_bits;                              //1 to 8
};
//------------------------------------------------------------------------------

#endif

//------------------------------------------------------------------------------
static_assert (sizeof (header_field) == sizeof (header_field::raw_type), "");
//------------------------------------------------------------------------------
enum field_class
{
    ufo_numeric,
    ufo_non_numeric,
};
//------------------------------------------------------------------------------
static const uword field_class_bits = 1;
//------------------------------------------------------------------------------
enum numeric_class
{
    ufo_integral,
    ufo_non_integral,
};
//------------------------------------------------------------------------------
static const uword numeric_class_bits = 1;
//------------------------------------------------------------------------------
enum non_integral_class
{
    ufo_double,
    ufo_float,
    ufo_bool,
};
//------------------------------------------------------------------------------
static const uword non_integral_class_bits = 2;
//------------------------------------------------------------------------------
enum non_numeric_class
{
    ufo_c_str,
    ufo_deep_copied_str,
    ufo_deep_copied_mem,
//    shared_ptr_str,
//    shared_ptr_mem,
};
//------------------------------------------------------------------------------
static const uword non_numeric_class_bits = 3;
//------------------------------------------------------------------------------
struct integral_field
{
    typedef u8 raw_type;

    enum original_type_values
    {
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
struct non_integral_field
{
    typedef u8 raw_type;
    static const uword bool_val_bits      = 1;

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
struct non_numeric_field
{
    typedef u8 raw_type;

    raw_type fclass                   : field_class_bits;
    raw_type nnclass                  : numeric_class_bits;
    raw_type deep_copied_length_bytes : numeric_bytes_bits;
};
//------------------------------------------------------------------------------
static_assert(
        sizeof (non_numeric_field) == sizeof (non_numeric_field::raw_type), ""
        );
//------------------------------------------------------------------------------
}} //namespaces

#endif /* UFO_LOG_FIELDS_HPP_ */
