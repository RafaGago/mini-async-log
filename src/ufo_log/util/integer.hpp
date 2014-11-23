/*
 * integer.hpp
 *
 *  Created on: Jun 3, 2014
 *      Author: rafgag
 */


#ifndef UFO_INTEGER_HPP_
#define UFO_INTEGER_HPP_

#ifndef UFO_USE_BOOST_CSTDINT

#include <cstdint>

namespace ufo {

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

namespace ufo {

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

namespace ufo {

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
struct integer_for_bytes <8>
{
    typedef u64 unsigned_type;
    typedef i64 signed_type;
};
//------------------------------------------------------------------------------
template <>
struct integer_for_bytes <4>
{
    typedef u32 unsigned_type;
    typedef i32 signed_type;
};
//------------------------------------------------------------------------------
template <>
struct integer_for_bytes <2>
{
    typedef u16 unsigned_type;
    typedef i16 signed_type;
};
//------------------------------------------------------------------------------
template <>
struct integer_for_bytes <1>
{
    typedef u8 unsigned_type;
    typedef i8 signed_type;
};
//------------------------------------------------------------------------------
typedef integer_for_bytes<sizeof (void*)>::signed_type   word;
typedef integer_for_bytes<sizeof (void*)>::unsigned_type uword;
//------------------------------------------------------------------------------

} //namespace ufo

#endif /* UFO_INTEGER_HPP_ */
