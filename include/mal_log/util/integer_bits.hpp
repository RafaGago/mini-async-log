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

#ifndef MAL_INTEGER_BITS_HPP_
#define MAL_INTEGER_BITS_HPP_

//overloads for integer_bits.h

//todo: add constexpr when widely supported

#include <type_traits>
#include <cassert>
#include <mal_log/util/integer.hpp>
#include <mal_log/util/integer_bits.h>
#include <mal_log/util/integer_bits_detail.hpp>

namespace mal {
//------------------------------------------------------------------------------
template <class T>
inline
T is_power2 (T x)
{
    static_assert (std::is_unsigned<T>::value, "Just for unsigned types");
    return (!(x & (x - 1)));
};
//------------------------------------------------------------------------------
template <class T>
inline
T div_ceil (T num, T den) { return ((num + den - 1) / den); };                  //beware of overflows (num + den)
//------------------------------------------------------------------------------
template <class T>
inline
T div_nearest (T num, T den) { return ((num + (den >> 1)) / den); };            //beware of overflows (num + (den >> 1))
//------------------------------------------------------------------------------
template <class T>
inline
T average (T x, T y) //saves a div and can't overflow.
{
    static_assert (std::is_unsigned<T>::value, "Just for unsigned types");
    return ((x & y) + ((x ^ y) >> 1));
};
//------------------------------------------------------------------------------
template <class T>
inline
T set_bit (uword bit_idx, T var)
{
    assert (bit_idx < (sizeof (T) * 8));
    var &= ((T) 1) << ((T) bit_idx);
    return var;
};
//------------------------------------------------------------------------------
template <class T>
inline
T clear_bit (uword bit_idx, T var)
{
    assert (bit_idx < (sizeof (T) * 8));
    var &= ~(((T) 1) << bit_idx);
    return var;
};
//------------------------------------------------------------------------------
template <class T>
inline
T set_bit_to (uword bit_idx, bool bit_val, T var)
{
    assert (bit_idx < (sizeof (T) * 8));
    var = clear_bit (bit_idx, var) | ((T) bit_val) << bit_idx;
    return var;
};
//------------------------------------------------------------------------------
template <class T>
inline
T get_bit (T var, uword bit_idx)
{
    assert (bit_idx < (sizeof (T) * 8));
    return  (((var) >> (bit_idx)) & 1);
};
//------------------------------------------------------------------------------
template <class T>
inline
bool get_bit_bool (T var, uword bit_idx)
{
    return (bool) get_bit (var, bit_idx);
};
//------------------------------------------------------------------------------
template <class T>
inline
void xor_swap(T a, T b) { return ((a) ^= (b), (b) ^= (a), (a) ^= (b)); };
//------------------------------------------------------------------------------
template <class T>
inline
T& xor_clr (T& val) { ((val) ^= (val)); return val; };
//------------------------------------------------------------------------------
template <class T>
inline
T ones_mask (T binary_ones_from_lsb_count)
{
    return ((1 << binary_ones_from_lsb_count) - 1);
};
//------------------------------------------------------------------------------
template <class T>
inline
bool is_multiple (T candidate, T value)
{
    return ((candidate / value) == div_ceil (candidate, value));
};
//------------------------------------------------------------------------------
template <class T>
inline
bool is_multiple_safe (T candidate, T value)
{
    return candidate && value && is_multiple (candidate, value);
};
//------------------------------------------------------------------------------
template <class T>
inline
bool are_multiple (T v1, T v2)
{
    return (v1 > v2) ? is_multiple (v1, v2) : is_multiple (v2, v1);
};
//------------------------------------------------------------------------------
template <class T>
inline
bool are_multiple_safe (T v1, T v2)
{
    return v1 && v2 && are_multiple (v1, v2);
};
//------------------------------------------------------------------------------
inline
int ones (uint64_t x) { return ones_64 (x); };

inline
int ones (uint32_t x) { return ones_32 (x); };

inline
int ones (uint16_t x) { return ones_16 (x); };

inline
int ones (uint8_t x) { return ones_8 (x); };
//------------------------------------------------------------------------------
inline
uint64_t set_from_msb_to_r (uint64_t x) { return set_from_msb_to_r_64 (x); };

inline
uint32_t set_from_msb_to_r (uint32_t x) { return set_from_msb_to_r_32 (x); };

inline
uint16_t set_from_msb_to_r (uint16_t x) { return set_from_msb_to_r_16 (x); };

inline
uint8_t set_from_msb_to_r (uint8_t x) { return set_from_msb_to_r_8 (x); };
//------------------------------------------------------------------------------
inline
uint64_t clear_non_msb (uint64_t x) { return clear_non_msb_64 (x); };

inline
uint32_t clear_non_msb (uint32_t x) { return clear_non_msb_32 (x); };

inline
uint16_t clear_non_msb (uint16_t x) { return clear_non_msb_16 (x); };

inline
uint8_t clear_non_msb (uint8_t x) { return clear_non_msb_8 (x); };
//------------------------------------------------------------------------------
inline
int log2_floor (uint64_t x) { return log2_floor_64 (x); };

inline
int log2_floor (uint32_t x) { return log2_floor_32 (x); };

inline
int log2_floor (uint16_t x) { return log2_floor_16 (x); };

inline
int log2_floor (uint8_t x) { return log2_floor_8 (x); };
//------------------------------------------------------------------------------
inline
int log2_ceil (uint64_t x) { return log2_ceil_64 (x); };

inline
int log2_ceil (uint32_t x) { return log2_ceil_32 (x); };

inline
int log2_ceil (uint16_t x) { return log2_ceil_16 (x); };

inline
int log2_ceil (uint8_t x) { return log2_ceil_8 (x); };
//------------------------------------------------------------------------------
inline
uint64_t next_pow2 (uint64_t x) { return next_pow2_64 (x); };

inline
uint32_t next_pow2 (uint32_t x) { return next_pow2_32 (x); };

inline
uint16_t next_pow2 (uint16_t x) { return next_pow2_16 (x); };

inline
uint8_t next_pow2 (uint8_t x) { return next_pow2_8 (x); };
//------------------------------------------------------------------------------
inline
uint64_t keep_highest_bit (uint64_t x) { return keep_highest_bit_64 (x); };

inline
uint32_t keep_highest_bit (uint32_t x) { return keep_highest_bit_32 (x); };

inline
uint16_t keep_highest_bit (uint16_t x) { return keep_highest_bit_16 (x); };

inline
uint8_t keep_highest_bit (uint8_t x) { return keep_highest_bit_8 (x); };
//------------------------------------------------------------------------------
inline
uword highest_used_byte (uint64_t x) { return highest_used_byte_64 (x); };

inline
uword highest_used_byte (uint32_t x) { return highest_used_byte_32 (x); };

inline
uword highest_used_byte (uint16_t x) { return highest_used_byte_16 (x); };

inline
uword highest_used_byte (uint8_t x) { return highest_used_byte_8 (x); };
//------------------------------------------------------------------------------
inline
uint64_t round_to_next_pow2 (uint64_t x)
{
    return is_power2 (x) ? x : next_pow2 (x);
};

inline
uint32_t round_to_next_pow2 (uint32_t x)
{
    return is_power2 (x) ? x : next_pow2 (x);
};

inline
uint16_t round_to_next_pow2 (uint16_t x)
{
    return is_power2 (x) ? x : next_pow2 (x);
};

inline
uint8_t round_to_next_pow2 (uint8_t x)
{
    return is_power2 (x) ? x : next_pow2 (x);
};
//------------------------------------------------------------------------------
inline
unsigned bit_array_write(
        uint8_t* buff,
        uint8_t  val,
        unsigned buff_bit_idx,
        unsigned val_bit_count
        )
{
    return bit_array_write_8 (buff, val, buff_bit_idx, val_bit_count);
};

inline
unsigned bit_array_write(
        uint8_t* buff,
        uint16_t val,
        unsigned buff_bit_idx,
        unsigned val_bit_count
        )
{
    return bit_array_write_16 (buff, val, buff_bit_idx, val_bit_count);
};

inline
unsigned bit_array_write(
        uint8_t* buff,
        uint32_t val,
        unsigned buff_bit_idx,
        unsigned val_bit_count
        )
{
    return bit_array_write_32 (buff, val, buff_bit_idx, val_bit_count);
};

inline
unsigned bit_array_write(
        uint8_t* buff,
        uint64_t val,
        unsigned buff_bit_idx,
        unsigned val_bit_count
        )
{
    return bit_array_write_64 (buff, val, buff_bit_idx, val_bit_count);
};
//------------------------------------------------------------------------------
inline
unsigned bit_array_read(
            uint8_t*       result,
            const uint8_t* buff,
            unsigned       buff_bit_idx,
            unsigned       bit_count
            )
{
    return bit_array_read_8 (result, buff, buff_bit_idx, bit_count);
};
//------------------------------------------------------------------------------
inline
unsigned bit_array_read(
            uint16_t*      result,
            const uint8_t* buff,
            unsigned       buff_bit_idx,
            unsigned       bit_count
            )
{
    return bit_array_read_16 (result, buff, buff_bit_idx, bit_count);
};
//------------------------------------------------------------------------------
inline
unsigned bit_array_read(
            uint32_t*      result,
            const uint8_t* buff,
            unsigned       buff_bit_idx,
            unsigned       bit_count
            )
{
    return bit_array_read_32 (result, buff, buff_bit_idx, bit_count);
};
//------------------------------------------------------------------------------
inline
unsigned bit_array_read(
            uint64_t*      result,
            const uint8_t* buff,
            unsigned       buff_bit_idx,
            unsigned       bit_count
            )
{
    return bit_array_read_64 (result, buff, buff_bit_idx, bit_count);
};
//--------------------------------------------------------------------
namespace detail {

    template<class T, unsigned bytes = sizeof (T)>
    struct reverse_bytes{};

    template<>
    struct reverse_bytes<uint16_t, 2>
    {
        static uint16_t result (uint16_t val)
        {
            return reverse_bytes_16 (val);
        };
    };

    template<>
    struct reverse_bytes<uint32_t, 3>
    {
        static uint32_t result (uint32_t val)
        {
            return reverse_bytes_24 (val);
        };
    };

    template<>
    struct reverse_bytes<uint32_t, 4>
    {
        static uint32_t result (uint32_t val)
        {
            return reverse_bytes_32 (val);
        };
    };

    template<>
    struct reverse_bytes<uint64_t, 5>
    {
        static uint64_t result (uint64_t val)
        {
            return reverse_bytes_40 (val);
        };
    };

    template<>
    struct reverse_bytes<uint64_t, 6>
    {
        static uint64_t result (uint64_t val)
        {
            return reverse_bytes_48 (val);
        };
    };

    template<>
    struct reverse_bytes<uint64_t, 7>
    {
        static uint64_t result (uint64_t val)
        {
            return reverse_bytes_56 (val);
        };
    };

    template<>
    struct reverse_bytes<uint64_t, 8>
    {
        static uint64_t result (uint64_t val)
        {
            return reverse_bytes_64 (val);
        };
    };

} //namespace detail
//------------------------------------------------------------------------------
template <class T>
static inline
T reverse_bytes (T val)
{
    return detail::reverse_bytes<T>::result (val);
};

template <uword bytes, class T>
static inline
T reverse_bytes (T val)
{
    return detail::reverse_bytes<T, bytes>::result (val);
};
//------------------------------------------------------------------------------

// The next functions portabily write integers to a byte array as "big endian"
// or "little endian".
//------------------------------------------------------------------------------
template <class T>
static inline
void byte_array_write_le (uint8_t* out, T val) //le = little endian
{
    detail::byte_array_write<T>::little_endian (out, val);
};
//------------------------------------------------------------------------------
template <uword byte_count, class T>
static inline
void byte_array_write_le (uint8_t* out, T val) //le = little endian
{
    detail::byte_array_write<T, byte_count>::little_endian (out, val);
};
//------------------------------------------------------------------------------
template <class T>
static inline
void byte_array_write_be (uint8_t* out, T val) //be = big endian
{
    detail::byte_array_write<T>::big_endian (out, val);
};
//------------------------------------------------------------------------------
template <uword byte_count, class T>
static inline
void byte_array_write_be (uint8_t* out, T val) //be = big endian
{
    detail::byte_array_write<T, byte_count>::big_endian (out, val);
};
//------------------------------------------------------------------------------
// The next functions portably read integers from a "big endian" or "little
// endian" byte array.
//------------------------------------------------------------------------------
template <uword byte_count>
static inline
typename detail::byte_array_read<byte_count>::uint_return_type  //will be the minimun integer type that can allocate n "bytes"
byte_array_read_le (const uint8_t* in)                          //le = little endian
{
    return detail::byte_array_read<byte_count>::little_endian (in);
};
//------------------------------------------------------------------------------
template <uword byte_count>
static inline
typename detail::byte_array_read<byte_count>::uint_return_type  //will be the minimun integer type that can allocate n "bytes"
byte_array_read_be (const uint8_t* in)                          //be = big endian
{
    return detail::byte_array_read<byte_count>::big_endian (in);
};
//--------------------------------------------------------------------
} //namespace

#endif /* INTEGER_BITS_HPP_ */
