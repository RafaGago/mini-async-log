/*
The BSD 3-clause license

Copyright (c) 2013-2014 Diadrom AB. All rights reserved.

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

THIS SOFTWARE IS PROVIDED BY DIADROM AB "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
SHALL DIADROM AB OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of Diadrom AB.
--------------------------------------------------------------------------------
*/


#ifndef MAL_INTEGER_HPP_
#define MAL_INTEGER_HPP_

#ifndef MAL_USE_BOOST_CSTDINT

#include <cstdint>

namespace mal {

typedef int8_t   int8;
typedef int16_t  int16;
typedef int32_t  int32;
typedef int64_t  int64;
typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

}

#else

#include <boost/cstdint.hpp>

namespace mal {

typedef boost::int8_t   int8;
typedef boost::int16_t  int16;
typedef boost::int32_t  int32;
typedef boost::int64_t  int64;
typedef boost::uint8_t  uint8;
typedef boost::uint16_t uint16;
typedef boost::uint32_t uint32;
typedef boost::uint64_t uint64;

}

#endif

namespace mal {

typedef int8   i8;
typedef int16  i16;
typedef int32  i32;
typedef int64  i64;

typedef int8   s8;
typedef int16  s16;
typedef int32  s32;
typedef int64  s64;

typedef uint8  u8;
typedef uint16 u16;
typedef uint32 u32;
typedef uint64 u64;

template <unsigned bytes>
struct integer_for_bytes
{
};
//------------------------------------------------------------------------------
template <>
struct integer_for_bytes <8> {
    typedef u64 unsigned_type;
    typedef i64 signed_type;
};
//------------------------------------------------------------------------------
template <>
struct integer_for_bytes <4> {
    typedef u32 unsigned_type;
    typedef i32 signed_type;
};
//------------------------------------------------------------------------------
template <>
struct integer_for_bytes <2> {
    typedef u16 unsigned_type;
    typedef i16 signed_type;
};
//------------------------------------------------------------------------------
template <>
struct integer_for_bytes <1> {
    typedef u8 unsigned_type;
    typedef i8 signed_type;
};
//------------------------------------------------------------------------------
typedef integer_for_bytes<sizeof (void*)>::signed_type   word;
typedef integer_for_bytes<sizeof (void*)>::unsigned_type uword;
//------------------------------------------------------------------------------

} //namespace mal

#endif /* MAL_INTEGER_HPP_ */
