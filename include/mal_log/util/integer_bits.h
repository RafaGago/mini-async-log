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

#ifndef MAL_INTEGER_BITS_AND_FUNCS_H_
#define MAL_INTEGER_BITS_AND_FUNCS_H_

//This file is a C header, don't use C++ features

#include <stdint.h>
#include <assert.h>

#if !defined (__cplusplus) || 1

// IF C++ CODE JUST USE THESE MACROS WHERE A CONSTANT EXPRESSION IS NEEDED!
// THERE AN EQUIVALENT TYPESAFE C++ HEADER

//todo: delete the "|| 1" above when constexpr is widely available and
//      add constexpr to the equivalent functions in integer_bits.hpp

#define IS_POWER2(x) (!((x) & ((x) - 1)))

#define DIV_CEIL(num, den) (((num) + (den) - 1) / (den))
#define DIV_NEAREST(num, den) (((num) + ((den) >> 1)) / (den))
    //saves a div and can't overflow.
#define UNSIGNED_AVERAGE(x, y) (((x) & (y)) + (((x) ^ (y)) >> 1))

#define SET_BIT_TO(variable, bit_idx, boolean)\
    ((variable) = ((variable) & (~(1 << (bit_idx)))) |\
                  (((boolean) & 1) << (bit_idx)))

#define GET_BIT(var, bit_idx) (((var) >> (bit_idx)) & 1)

#define GET_BYTE(var, byte_idx) ((var >> (byte_idx * 8)) & 255)

#define XOR_SWAP(a, b) ((a) ^= (b), (b) ^= (a), (a) ^= (b))
#define XOR_CLR(a, b) ((a) ^= (a))

#define ONES_MASK(binary_ones_from_lsb_count)\
    ((1 << (binary_ones_from_lsb_count)) - 1)

#define IS_MULTIPLE(candidate, amount) (((candidate) % (amount)) == 0)

#define IS_MULTIPLE_SAFE(candidate, amount)\
    ((candidate) && (amount) && (IS_MULTIPLE ((candidate), (amount))))

#define ARE_MULTIPLE(val1, val2)\
    ((val1 > val2) ? IS_MULTIPLE (val1, val2) : IS_MULTIPLE (val2, val1))

#define ARE_MULTIPLE_SAFE(val1, val2)\
    ((val1) && (val2) && (ARE_MULTIPLE ((val1), (val2))))

#endif

#if defined (__cplusplus)
namespace mal {
#endif

// todo: if gcc, have a look at
// __builtin_popcount (unsigned),
// __builtin_popcountl (unsigned long),
// __builtin_popcountll (unsigned long long),
//--------------------------------------------------------------------
static inline
int ones_64 (uint64_t x) //Hamming weight, population count
{
    const uint64_t m1  = 0x5555555555555555ULL; //all 01010101 ...
    const uint64_t m2  = 0x3333333333333333ULL; //all 00110011 ...
    const uint64_t m4  = 0x0f0f0f0f0f0f0f0fULL; //all 00001111 ...

    x -= (x >> 1) & m1;
    x = (x & m2) + ((x >> 2) & m2);
    x = (x + (x >> 4)) & m4;
    x += x >>  8;
    x += x >> 16;
    x += x >> 32;
    return (int) (x & 0x7f);
}
//--------------------------------------------------------------------
static inline
int ones_32 (uint32_t x) //Hamming weight, population count
{
    const uint32_t m1  = 0x55555555ULL;
    const uint32_t m2  = 0x33333333ULL;
    const uint32_t m4  = 0x0f0f0f0fULL;

    x -= (x >> 1) & m1;
    x = (x & m2) + ((x >> 2) & m2);
    x = (x + (x >> 4)) & m4;
    x += x >>  8;
    x += x >> 16;
    return (int) (x & 0x3f);
}
//--------------------------------------------------------------------
static inline
int ones_16 (uint16_t x) //Hamming weight, population count
{
    const uint16_t m1  = 0x5555ULL;
    const uint16_t m2  = 0x3333ULL;
    const uint16_t m4  = 0x0f0fULL;

    x -= (x >> 1) & m1;
    x = (x & m2) + ((x >> 2) & m2);
    x = (x + (x >> 4)) & m4;
    x += x >>  8;
    return (int) (x & 0x1f);
}
//--------------------------------------------------------------------
static inline
int ones_8 (uint8_t x) //Hamming weight, population count
{
    const uint8_t m1  = 0x55;
    const uint8_t m2  = 0x33;
    const uint8_t m4  = 0x0f;

    x -= (x >> 1) & m1;
    x = (x & m2) + ((x >> 2) & m2);
    x = (x + (x >> 4)) & m4;
    return (int) (x & 0x0f);
}
//------------------------------------------------------------------------------
static inline
uint64_t set_from_msb_to_r_64 (uint64_t x) //sets all bits from msb to the right
{
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    x |= (x >> 32);
    return x;
}
//------------------------------------------------------------------------------
static inline
uint32_t set_from_msb_to_r_32 (uint32_t x) //sets all bits from msb to the right
{
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    return x;
}
//------------------------------------------------------------------------------
static inline
uint16_t set_from_msb_to_r_16 (uint16_t x) //sets all bits from msb to the right
{
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    return x;
}
//------------------------------------------------------------------------------
static inline
uint8_t set_from_msb_to_r_8 (uint8_t x) //sets all bits from msb to the right
{
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    return x;
}
//------------------------------------------------------------------------------
static inline
uint64_t clear_non_msb_64 (uint64_t x) //can overflow
{
    x = set_from_msb_to_r_64 (x);
    return x & (~(x >> 1));
}
//------------------------------------------------------------------------------
static inline
uint32_t clear_non_msb_32 (uint32_t x) //can overflow
{
    x = set_from_msb_to_r_32 (x);
    return x & (~(x >> 1));
}
//------------------------------------------------------------------------------
static inline
uint16_t clear_non_msb_16 (uint16_t x) //can overflow
{
    x = set_from_msb_to_r_16 (x);
    return x & (~(x >> 1));
}
//------------------------------------------------------------------------------
static inline
uint8_t clear_non_msb_8 (uint8_t x) //can overflow
{
    x = set_from_msb_to_r_8 (x);
    return x & (~(x >> 1));
}
//------------------------------------------------------------------------------
static inline
int log2_floor_64 (uint64_t x) //log_2 of 0 will give -1
{
    x = set_from_msb_to_r_64 (x);
    return ones_64 (x) - 1;
}
//------------------------------------------------------------------------------
static inline
uint32_t log2_floor_32 (uint32_t x) //log_2 of 0 will give -1
{
    x = set_from_msb_to_r_32 (x);
    return ones_32 (x) - 1;
}
//------------------------------------------------------------------------------
static inline
int log2_floor_16 (uint16_t x) //log_2 of 0 will give -1
{
    x = set_from_msb_to_r_16 (x);
    return ones_16 (x) - 1;
}
//------------------------------------------------------------------------------
static inline
int log2_floor_8 (uint8_t x) //log_2 of 0 will give -1
{
    x = set_from_msb_to_r_8 (x);
    return ones_8 (x) - 1;
}
//------------------------------------------------------------------------------
static inline
int log2_ceil_64 (uint64_t x) //log_2 of 0 will give -1
{
    int64_t y = x & (x - 1);
    y |= -y;
    y >>= ((sizeof (y) * 8) - 1);

    x = set_from_msb_to_r_64 (x);
    return ones_64 (x) - 1 - (int) y;
}
//------------------------------------------------------------------------------
static inline
int log2_ceil_32 (uint32_t x) //log_2 of 0 will give -1
{
    int32_t y = x & (x - 1);
    y |= -y;
    y >>= ((sizeof (y) * 8) - 1);

    x = set_from_msb_to_r_32 (x);
    return ones_32 (x) - 1 - (int) y;
}
//------------------------------------------------------------------------------
static inline
int log2_ceil_16 (uint16_t x) //log_2 of 0 will give -1
{
    int16_t y = x & (x - 1);
    y |= -y;
    y >>= ((sizeof (y) * 8) - 1);

    x = set_from_msb_to_r_16 (x);
    return ones_16 (x) - 1 - (int) y;
}
//------------------------------------------------------------------------------
static inline
int log2_ceil_8 (uint8_t x) //log_2 of 0 will give -1
{
    int8_t y = x & (x - 1);
    y |= -y;
    y >>= ((sizeof (y) * 8) - 1);

    x = set_from_msb_to_r_8 (x);
    return ones_8 (x) - 1 - (int) y;
}
//------------------------------------------------------------------------------
static inline
uint64_t next_pow2_64 (uint64_t x) //can overflow
{
    x = set_from_msb_to_r_64 (x);
    return x + 1;
}
//------------------------------------------------------------------------------
static inline
uint32_t next_pow2_32 (uint32_t x) //can overflow
{
    x = set_from_msb_to_r_32 (x);
    return x + 1;
}
//------------------------------------------------------------------------------
static inline
uint16_t next_pow2_16 (uint16_t x) //can overflow
{
    x = set_from_msb_to_r_16 (x);
    return x + 1;
}
//------------------------------------------------------------------------------
static inline
uint8_t next_pow2_8 (uint8_t x) //can overflow
{
    x = set_from_msb_to_r_8 (x);
    return x + 1;
}
//------------------------------------------------------------------------------
static inline
uint64_t keep_highest_bit_64 (uint64_t x)
{
    x = set_from_msb_to_r_64 (x);
    return x - (x >> 1);
}
//------------------------------------------------------------------------------
static inline
uint32_t keep_highest_bit_32 (uint32_t x)
{
    x = set_from_msb_to_r_32 (x);
    return x - (x >> 1);
}
//------------------------------------------------------------------------------
static inline
uint16_t keep_highest_bit_16 (uint16_t x)
{
    x = set_from_msb_to_r_16 (x);
    return x - (x >> 1);
}
//------------------------------------------------------------------------------
static inline
uint8_t keep_highest_bit_8 (uint8_t x)
{
    x = set_from_msb_to_r_8 (x);
    return x - (x >> 1);
}
//------------------------------------------------------------------------------
static inline
unsigned highest_used_byte_8 (uint8_t v)
{
    return 0;
}
//------------------------------------------------------------------------------
static inline
unsigned highest_used_byte_16 (uint16_t v)
{
    return (unsigned (v > 255));
}
//------------------------------------------------------------------------------
static inline
unsigned highest_used_byte_32 (uint32_t v)
{
#ifdef MAL_INTEGER_BITS_NO_BRANCHING
    unsigned res = 0, cmp = 0;

    cmp = ((unsigned) ((v & 0x0000ff00ULL) == 0)) - 1;
    res = (res & ~cmp) | (cmp & 1);

    cmp = ((unsigned) ((v & 0x00ff0000ULL) == 0)) - 1;
    res = (res & ~cmp) | (cmp & 2);

    cmp = ((unsigned) ((v & 0xff000000ULL) == 0)) - 1;
    res = (res & ~cmp) | (cmp & 3);

    return res;
#else
    if ((v & 0xffffff00ULL) == 0) { return 0; }
    if ((v & 0xffff0000ULL) == 0) { return 1; }
    if ((v & 0xff000000ULL) == 0) { return 2; }
    return 3;
#endif

}
//------------------------------------------------------------------------------
static inline
unsigned highest_used_byte_64 (uint64_t v)
{
#ifdef MAL_INTEGER_BITS_NO_BRANCHING
    unsigned res = 0, cmp = 0;

    cmp = ((unsigned) ((v & 0x000000000000ff00ULL) == 0)) - 1;
    res = (res & ~cmp) | (cmp & 1);

    cmp = ((unsigned) ((v & 0x0000000000ff0000ULL) == 0)) - 1;
    res = (res & ~cmp) | (cmp & 2);

    cmp = ((unsigned) ((v & 0x00000000ff000000ULL) == 0)) - 1;
    res = (res & ~cmp) | (cmp & 3);

    cmp = ((unsigned) ((v & 0x000000ff00000000ULL) == 0)) - 1;
    res = (res & ~cmp) | (cmp & 4);

    cmp = ((unsigned) ((v & 0x0000ff0000000000ULL) == 0)) - 1;
    res = (res & ~cmp) | (cmp & 5);

    cmp = ((unsigned) ((v & 0x00ff000000000000ULL) == 0)) - 1;
    res = (res & ~cmp) | (cmp & 6);

    cmp = ((unsigned) ((v & 0xff00000000000000ULL) == 0)) - 1;
    res = (res & ~cmp) | (cmp & 7);

    return res;
#else
    if ((v & 0xffffffffffffff00ULL) == 0) { return 0; }
    if ((v & 0xffffffffffff0000ULL) == 0) { return 1; }
    if ((v & 0xffffffffff000000ULL) == 0) { return 2; }
    if ((v & 0xffffffff00000000ULL) == 0) { return 3; }
    if ((v & 0xffffff0000000000ULL) == 0) { return 4; }
    if ((v & 0xffff000000000000ULL) == 0) { return 5; }
    if ((v & 0xff00000000000000ULL) == 0) { return 6; }
    return 7;
#endif
}
//------------------------------------------------------------------------------

#ifndef __cplusplus
//------------------------------------------------------------------------------
static inline
uint64_t round_to_next_pow2_64 (uint64_t x) //can overflow
{
    return IS_POWER2(x) ? x : next_pow2_64 (x);
}
//------------------------------------------------------------------------------
static inline
uint32_t round_to_next_pow2_32 (uint32_t x) //can overflow
{
    return IS_POWER2(x) ? x : next_pow2_32 (x);
}
//------------------------------------------------------------------------------
static inline
uint16_t round_to_next_pow2_16 (uint16_t x) //can overflow
{
    return IS_POWER2(x) ? x : next_pow2_16 (x);
}
//------------------------------------------------------------------------------
static inline
uint8_t round_to_next_pow2_8 (uint8_t x) //can overflow
{
    return IS_POWER2(x) ? x : next_pow2_8 (x);
}

#endif
//------------------------------------------------------------------------------
//writes n bits to a byte respecting the other bit values
static inline
void bitvar_set (uint8_t* dest, uint8_t val, unsigned val_bits_count)
{
    assert (val_bits_count <= 8);

    uint8_t mask = ((1 << val_bits_count) - 1);
    *dest &= ~mask; //AND clear destiny bits
    *dest |= (val & mask); //OR set
};
//------------------------------------------------------------------------------
//writes n bits to a byte respecting the other bit values, offset version
static inline
void bitvar_set_offset(
        uint8_t* dest,
        uint8_t  val,
        unsigned val_bits_count,
        unsigned dest_bit_idx
        )
{
    assert (dest_bit_idx < 8);
    assert (val_bits_count <= (8 - dest_bit_idx));

    uint8_t tail_mask = 255 << (dest_bit_idx + val_bits_count);
    *dest &= (((1 << dest_bit_idx) - 1) | tail_mask); //AND clear destiny bits
    *dest |= ((val & ((1 << val_bits_count) - 1)) << dest_bit_idx) & ~tail_mask; //OR set
};
//------------------------------------------------------------------------------
//read bits from a byte (this one is mask and it wouldn't need a function)
static inline
void bitvar_get (uint8_t* result, uint8_t val, unsigned val_bits_count)
{
    assert (val_bits_count <= 8);
    assert (val_bits_count);

    *result = val & ((1 << val_bits_count) - 1);
};
//------------------------------------------------------------------------------
//read bits from a byte, offset version
static inline
void bitvar_get_offset(
        uint8_t* result,
        uint8_t  val,
        unsigned val_bits_count,
        unsigned val_bit_idx
        )
{
    assert (val_bit_idx < 8);
    assert (val_bits_count <= (8 - val_bit_idx));
    assert (val_bits_count);

    *result = (val >> val_bit_idx) & ((1 << val_bits_count) - 1);
};
//------------------------------------------------------------------------------
//writes bits to a byte array of bits, buff is unchecked, user should ensure that buff_bit_idx + val_bit_count doesn't go out of bounds
static inline
unsigned bit_array_write_8(
        uint8_t* buff,
        uint8_t  val,
        unsigned buff_bit_idx,
        unsigned val_bit_count
        )
{
    assert (val_bit_count <= 8);

    unsigned buff_idx = buff_bit_idx / 8;
    unsigned first_offset = buff_bit_idx & 7;
    unsigned bits_first = ((8 - first_offset) < val_bit_count) ?
                                (8 - first_offset) : val_bit_count;
    unsigned bits_second = val_bit_count - bits_first;

    bitvar_set_offset (buff + buff_idx, val, bits_first, first_offset);

    if (bits_second) { //we need to write the next array position
        bitvar_set (buff + buff_idx + 1, val >> bits_first, bits_second);
    }
    return buff_bit_idx + val_bit_count;
};
//------------------------------------------------------------------------------
//writes bits to a byte array of bits, buff is unchecked, user should ensure that buff_bit_idx + val_bit_count doesn't go out of bounds
static inline
unsigned bit_array_write_16(
        uint8_t* buff,
        uint16_t val,
        unsigned buff_bit_idx,
        unsigned val_bit_count
        )
{
    bool two_steps = (val_bit_count > 8);

    unsigned buff_idx = bit_array_write_8(
                            buff,
                            (uint8_t) (val & ((1 << 8) - 1)),
                            buff_bit_idx,
                            (two_steps) ? 8 : val_bit_count
                            );
    if (two_steps) {
        buff_idx = bit_array_write_8(
                    buff,
                    (uint8_t) (val >> 8),
                    buff_idx,
                    val_bit_count - 8
                    );
    }
    return buff_idx;
};
//------------------------------------------------------------------------------
//writes bits to a byte array of bits, buff is unchecked, user should ensure that buff_bit_idx + val_bit_count doesn't go out of bounds
static inline
unsigned bit_array_write_32(
        uint8_t* buff,
        uint32_t val,
        unsigned buff_bit_idx,
        unsigned val_bit_count
        )
{
    bool two_steps = (val_bit_count > 16);

    unsigned buff_idx = bit_array_write_16(
                            buff,
                            (uint16_t) (val & ((1 << 16) - 1)),
                            buff_bit_idx,
                            (two_steps) ? 16 : val_bit_count
                            );
    if (two_steps) {
        buff_idx = bit_array_write_16(
                    buff,
                    (uint16_t) (val >> 16),
                    buff_bit_idx,
                    val_bit_count - 16
                    );
    }
    return buff_idx;
};
//------------------------------------------------------------------------------
//writes bits to a byte array of bits, buff is unchecked, user should ensure that buff_bit_idx + val_bit_count doesn't go out of bounds
static inline
unsigned bit_array_write_64(
        uint8_t* buff,
        uint64_t val,
        unsigned buff_bit_idx,
        unsigned val_bit_count
        )
{
    bool two_steps = (val_bit_count > 32);

    unsigned buff_idx = bit_array_write_32(
                            buff,
                            (uint32_t) (val & ((1ULL << 32) - 1)),
                            buff_bit_idx,
                            (two_steps) ? 32 : val_bit_count
                            );
    if (two_steps) {
        buff_idx = bit_array_write_32(
                        buff,
                        (uint32_t) (val >> 32),
                        buff_bit_idx,
                        val_bit_count - 32
                        );
    }
    return buff_idx;
};
//------------------------------------------------------------------------------
static inline
unsigned bit_array_read_8(
        uint8_t*       result,
        const uint8_t* buff,
        unsigned       buff_bit_idx,
        unsigned       bit_count
        )
{
    assert (bit_count <= 8);

    unsigned buff_idx = buff_bit_idx / 8;
    unsigned first_offset = buff_bit_idx & 7;
    unsigned bits_first = ((8 - first_offset) < bit_count) ?
                                (8 - first_offset) : bit_count;
    unsigned bits_second = bit_count - bits_first;

    bitvar_get_offset (result, buff[buff_idx], bits_first, first_offset);

    if (bits_second) { //we need to write the next array position
        uint8_t val;
        bitvar_get (&val, buff[buff_idx + 1], bits_second);
        *result |= val << bits_first;
    }
    return buff_bit_idx + bit_count;
};
//------------------------------------------------------------------------------
static inline
unsigned bit_array_read_16(
        uint16_t*      result,
        const uint8_t* buff,
        unsigned       buff_bit_idx,
        unsigned       bit_count
        )
{
    bool two_steps = (bit_count > 8);

    uint8_t acum;

    unsigned buff_idx = bit_array_read_8(
                        &acum, buff, buff_bit_idx, (two_steps) ? 8 : bit_count
                        );

    *result = (uint16_t) acum;

    if (two_steps) {
        buff_idx = bit_array_read_8 (&acum, buff, buff_idx, bit_count - 8);
        *result |= ((uint16_t) acum) << 8;
    }
    return buff_idx;
};
//------------------------------------------------------------------------------
static inline
unsigned bit_array_read_32(
        uint32_t*      result,
        const uint8_t* buff,
        unsigned       buff_bit_idx,
        unsigned       bit_count
        )
{
    bool two_steps = (bit_count > 16);

    uint16_t acum;

    unsigned buff_idx = bit_array_read_16(
                        &acum, buff, buff_bit_idx, (two_steps) ? 16 : bit_count
                        );
    *result = (uint32_t) acum;

    if (two_steps) {
        buff_idx = bit_array_read_16 (&acum, buff, buff_idx, bit_count - 16);
        *result |= ((uint32_t) acum) << 16;
    }
    return buff_idx;
};
//------------------------------------------------------------------------------
static inline
unsigned bit_array_read_64(
        uint64_t*      result,
        const uint8_t* buff,
        unsigned       buff_bit_idx,
        unsigned       bit_count
        )
{
    bool two_steps = (bit_count > 32);

    uint32_t acum;

    unsigned buff_idx = bit_array_read_32(
                        &acum, buff, buff_bit_idx, (two_steps) ? 32 : bit_count
                        );
    *result = (uint64_t) acum;

    if (two_steps) {
        buff_idx = bit_array_read_32 (&acum, buff, buff_idx, bit_count - 32);
        *result |= ((uint64_t) acum) << 32;
    }
    return buff_idx;
};
//------------------------------------------------------------------------------
static inline
uint8_t reverse_bits (uint8_t val)
{
    return (uint8_t)(
            ((((uint64_t) val) * 0x80200802ULL) & 0x0884422110ULL)
            * 0x0101010101ULL >> 32
            );
};
//------------------------------------------------------------------------------
static inline
uint16_t reverse_bytes_16 (uint16_t val)
{
    return (val >> 8) | (val << 8);
};
//------------------------------------------------------------------------------
static inline
uint32_t reverse_bytes_24 (uint32_t val)
{
    return ((val >> 16) & 255) | ((val << 16) & (255 << 16)) |
            (val & 0xff00ff00);
};
//------------------------------------------------------------------------------
static inline
uint32_t reverse_bytes_32 (uint32_t val)
{
    return (val << 24) | ((val << 8) & (255 << 16)) |
           (val >> 24) | ((val >> 8) & (255 << 8));
};
//------------------------------------------------------------------------------
static inline
uint64_t reverse_bytes_40 (uint64_t val)
{
    static const uint64_t b = 255ULL; //b = byte, just to save space
    return ((val << 32) & (b << 32)) | ((val << 16) & (b << 24)) |
           ((val >> 32) & b)         | ((val >> 16) & (b << 8))  |
           (val & 0xffffff0000ff0000) ;
};
//------------------------------------------------------------------------------
static inline
uint64_t reverse_bytes_48 (uint64_t val)
{
    static const uint64_t b = 255ULL; //b = byte, just to save space
    return ((val << 40) & (b << 40)) | ((val << 24) & (b << 32)) |
           ((val << 8) & (b << 24))  | ((val >> 40) & b) |
           ((val >> 24) & (b << 8))  | ((val >> 8) & (b << 16)) |
           (val & 0xffff000000000000);
};
//------------------------------------------------------------------------------
static inline
uint64_t reverse_bytes_56 (uint64_t val)
{
    static const uint64_t b = 255ULL; //b = byte, just to save space
    return ((val << 48) & (b << 48)) | ((val << 32) & (b << 40)) |
           ((val << 16) & (b << 32)) | ((val >> 48) & b) |
           ((val >> 32) & (b << 8))  | ((val >> 16) & (b << 16)) |
           (val & 0xff000000ff000000);
};
//------------------------------------------------------------------------------
static inline
uint64_t reverse_bytes_64 (uint64_t val)
{
    static const uint64_t b = 255ULL; //b = byte, just to save space
    return (val << 56) | ((val << 40) & (b << 48)) | ((val << 24) & (b << 40)) |
           ((val << 8) & (b << 32)) | (val >> 56) | ((val >> 40) & (b << 8))  |
           ((val >> 24) & (b << 16)) | ((val >> 8) & (b << 24));
};
//------------------------------------------------------------------------------
static inline
void byte_array_write_le_16 (uint8_t* out, uint16_t val) //le = little endian
{
    assert (out);
    out[0] = (uint8_t) val;
    out[1] = (uint8_t) (val >> 8);
}
//------------------------------------------------------------------------------
static inline
void byte_array_write_be_16 (uint8_t* out, uint16_t val) //be = big endian
{
    assert (out);
    out[0] = (uint8_t) (val >> 8);
    out[1] = (uint8_t) val;
}
//------------------------------------------------------------------------------
static inline
void byte_array_write_le_24 (uint8_t* out, uint32_t val) //le = little endian
{
    assert (out);
    out[0] = (uint8_t) val;
    out[1] = (uint8_t) (val >> 8);
    out[2] = (uint8_t) (val >> 16);
}
//------------------------------------------------------------------------------
static inline
void byte_array_write_be_24 (uint8_t* out, uint32_t val) //be = big endian
{
    assert (out);
    out[0] = (uint8_t) (val >> 16);
    out[1] = (uint8_t) (val >> 8);
    out[2] = (uint8_t) val;
}
//------------------------------------------------------------------------------
static inline
void byte_array_write_le_32 (uint8_t* out, uint32_t val) //le = little endian
{
    assert (out);
    out[0] = (uint8_t) val;
    out[1] = (uint8_t) (val >> 8);
    out[2] = (uint8_t) (val >> 16);
    out[3] = (uint8_t) (val >> 24);
}
//------------------------------------------------------------------------------
static inline
void byte_array_write_be_32 (uint8_t* out, uint32_t val) //be = big endian
{
    assert (out);
    out[0] = (uint8_t) (val >> 24);
    out[1] = (uint8_t) (val >> 16);
    out[2] = (uint8_t) (val >> 8);
    out[3] = (uint8_t) val;
}
//------------------------------------------------------------------------------
static inline
void byte_array_write_le_40 (uint8_t* out, uint64_t val) //le = little endian
{
    assert (out);
    out[0] = (uint8_t) val;
    out[1] = (uint8_t) (val >> 8);
    out[2] = (uint8_t) (val >> 16);
    out[3] = (uint8_t) (val >> 24);
    out[4] = (uint8_t) (val >> 32);
}
//------------------------------------------------------------------------------
static inline
void byte_array_write_be_40 (uint8_t* out, uint64_t val) //be = big endian
{
    assert (out);
    out[0] = (uint8_t) (val >> 32);
    out[1] = (uint8_t) (val >> 24);
    out[2] = (uint8_t) (val >> 16);
    out[3] = (uint8_t) (val >> 8);
    out[4] = (uint8_t) val;
}
//------------------------------------------------------------------------------
static inline
void byte_array_write_le_48 (uint8_t* out, uint64_t val) //le = little endian
{
    assert (out);
    out[0] = (uint8_t) val;
    out[1] = (uint8_t) (val >> 8);
    out[2] = (uint8_t) (val >> 16);
    out[3] = (uint8_t) (val >> 24);
    out[4] = (uint8_t) (val >> 32);
    out[5] = (uint8_t) (val >> 40);
}
//------------------------------------------------------------------------------
static inline
void byte_array_write_be_48 (uint8_t* out, uint64_t val) //be = big endian
{
    assert (out);
    out[0] = (uint8_t) (val >> 40);
    out[1] = (uint8_t) (val >> 32);
    out[2] = (uint8_t) (val >> 24);
    out[3] = (uint8_t) (val >> 16);
    out[4] = (uint8_t) (val >> 8);
    out[5] = (uint8_t) val;
}
//------------------------------------------------------------------------------
static inline
void byte_array_write_le_56 (uint8_t* out, uint64_t val) //le = little endian
{
    assert (out);
    out[0] = (uint8_t) val;
    out[1] = (uint8_t) (val >> 8);
    out[2] = (uint8_t) (val >> 16);
    out[3] = (uint8_t) (val >> 24);
    out[4] = (uint8_t) (val >> 32);
    out[5] = (uint8_t) (val >> 40);
    out[6] = (uint8_t) (val >> 48);
}
//------------------------------------------------------------------------------
static inline
void byte_array_write_be_56 (uint8_t* out, uint64_t val) //be = big endian
{
    assert (out);
    out[0] = (uint8_t) (val >> 48);
    out[1] = (uint8_t) (val >> 40);
    out[2] = (uint8_t) (val >> 32);
    out[3] = (uint8_t) (val >> 24);
    out[4] = (uint8_t) (val >> 16);
    out[5] = (uint8_t) (val >> 8);
    out[6] = (uint8_t) val;
}
//------------------------------------------------------------------------------
static inline
void byte_array_write_le_64 (uint8_t* out, uint64_t val) //le = little endian
{
    assert (out);
    out[0] = (uint8_t) val;
    out[1] = (uint8_t) (val >> 8);
    out[2] = (uint8_t) (val >> 16);
    out[3] = (uint8_t) (val >> 24);
    out[4] = (uint8_t) (val >> 32);
    out[5] = (uint8_t) (val >> 40);
    out[6] = (uint8_t) (val >> 48);
    out[7] = (uint8_t) (val >> 56);
}
//------------------------------------------------------------------------------
static inline
void byte_array_write_be_64 (uint8_t* out, uint64_t val) //be = big endian
{
    assert (out);
    out[0] = (uint8_t) (val >> 56);
    out[1] = (uint8_t) (val >> 48);
    out[2] = (uint8_t) (val >> 40);
    out[3] = (uint8_t) (val >> 32);
    out[4] = (uint8_t) (val >> 24);
    out[5] = (uint8_t) (val >> 16);
    out[6] = (uint8_t) (val >> 8);
    out[7] = (uint8_t) val;
}
//------------------------------------------------------------------------------
static inline
uint16_t byte_array_read_le_16 (const uint8_t* in) //le = little endian
{
    assert (in);
    return ((uint16_t) in[0]) | ((uint16_t) in[1] << 8);
}
//------------------------------------------------------------------------------
static inline
uint16_t byte_array_read_be_16 (const uint8_t* in) //be = big endian
{
    assert (in);
    return ((uint16_t) in[1]) | ((uint16_t) in[0] << 8);
}
//------------------------------------------------------------------------------
static inline
uint32_t byte_array_read_le_24 (const uint8_t* in) //le = little endian
{
    assert (in);
    return ((uint32_t) in[0]) | ((uint32_t) in[1] << 8) |
           ((uint32_t) in[2] << 16);
}
//------------------------------------------------------------------------------
static inline
uint32_t byte_array_read_be_24 (const uint8_t* in) //be = big endian
{
    return ((uint32_t) in[3]) | ((uint32_t) in[2] << 8) |
           ((uint32_t) in[1] << 16);
}
//------------------------------------------------------------------------------
static inline
uint32_t byte_array_read_le_32 (const uint8_t* in) //le = little endian
{
    assert (in);
    return ((uint32_t) in[0]) | ((uint32_t) in[1] << 8) |
           ((uint32_t) in[2] << 16) | ((uint32_t) in[3] << 24);
}
//------------------------------------------------------------------------------
static inline
uint32_t byte_array_read_be_32 (const uint8_t* in) //be = big endian
{
    assert (in);
    return ((uint32_t) in[3]) | ((uint32_t) in[2] << 8) |
           ((uint32_t) in[1] << 16) | ((uint32_t) in[0] << 24);
}
//------------------------------------------------------------------------------
static inline
uint64_t byte_array_read_le_40 (const uint8_t* in) //le = little endian
{
    assert (in);
    return ((uint64_t) in[0])       | ((uint64_t) in[1] << 8) |
           ((uint64_t) in[2] << 16) | ((uint64_t) in[3] << 24) |
           ((uint64_t) in[4] << 32);
}
//------------------------------------------------------------------------------
static inline
uint64_t byte_array_read_be_40 (const uint8_t* in) //be = big endian
{
    assert (in);
    return ((uint64_t) in[7])       | ((uint64_t) in[6] << 8) |
           ((uint64_t) in[5] << 16) | ((uint64_t) in[4] << 24) |
           ((uint64_t) in[3] << 32);
}
//------------------------------------------------------------------------------
static inline
uint64_t byte_array_read_le_48 (const uint8_t* in) //le = little endian
{
    assert (in);
    return ((uint64_t) in[0])       | ((uint64_t) in[1] << 8) |
           ((uint64_t) in[2] << 16) | ((uint64_t) in[3] << 24) |
           ((uint64_t) in[4] << 32) | ((uint64_t) in[5] << 40);
}
//------------------------------------------------------------------------------
static inline
uint64_t byte_array_read_be_48 (const uint8_t* in) //be = big endian
{
    assert (in);
    return ((uint64_t) in[7])       | ((uint64_t) in[6] << 8) |
           ((uint64_t) in[5] << 16) | ((uint64_t) in[4] << 24) |
           ((uint64_t) in[3] << 32) | ((uint64_t) in[2] << 40);
}
//------------------------------------------------------------------------------
static inline
uint64_t byte_array_read_le_56 (const uint8_t* in) //le = little endian
{
    assert (in);
    return ((uint64_t) in[0])       | ((uint64_t) in[1] << 8) |
           ((uint64_t) in[2] << 16) | ((uint64_t) in[3] << 24) |
           ((uint64_t) in[4] << 32) | ((uint64_t) in[5] << 40) |
           ((uint64_t) in[6] << 48);
}
//------------------------------------------------------------------------------
static inline
uint64_t byte_array_read_be_56 (const uint8_t* in) //be = big endian
{
    assert (in);
    return ((uint64_t) in[7])       | ((uint64_t) in[6] << 8) |
           ((uint64_t) in[5] << 16) | ((uint64_t) in[4] << 24) |
           ((uint64_t) in[3] << 32) | ((uint64_t) in[2] << 40) |
           ((uint64_t) in[1] << 48);
}
//------------------------------------------------------------------------------
static inline
uint64_t byte_array_read_le_64 (const uint8_t* in) //le = little endian
{
    assert (in);
    return ((uint64_t) in[0])       | ((uint64_t) in[1] << 8) |
           ((uint64_t) in[2] << 16) | ((uint64_t) in[3] << 24) |
           ((uint64_t) in[4] << 32) | ((uint64_t) in[5] << 40) |
           ((uint64_t) in[6] << 48) | ((uint64_t) in[7] << 56);
}
//------------------------------------------------------------------------------
static inline
uint64_t byte_array_read_be_64 (const uint8_t* in) //be = big endian
{
    assert (in);
    return ((uint64_t) in[7])       | ((uint64_t) in[6] << 8) |
           ((uint64_t) in[5] << 16) | ((uint64_t) in[4] << 24) |
           ((uint64_t) in[3] << 32) | ((uint64_t) in[2] << 40) |
           ((uint64_t) in[1] << 48) | ((uint64_t) in[0] << 56);
}

#if defined (__cplusplus)
} //namespace mal {
#endif

#ifndef __cplusplus

#include <limits.h> //todo: windows workaround non resorting to C++ if possible

#if (UINT_MAX == 0xff)

    #define ones ones_8
    #define set_from_msb_to_r set_from_msb_to_r_8
    #define log2_floor log2_floor_8
    #define log2_ceil log2_ceil_8
    #define next_pow2 next_pow2_8
    #define round_to_next_pow2 round_to_next_pow2_8
    #define clear_non_msb clear_non_msb_8

#elif  (UINT_MAX == 0xffffULL)

    #define ones ones_16
    #define set_from_msb_to_r set_from_msb_to_r_16
    #define log2_floor log2_floor_16
    #define log2_ceil log2_ceil_16
    #define next_pow2 next_pow2_16
    #define round_to_next_pow2 round_to_next_pow2_16
    #define clear_non_msb clear_non_msb_16

#elif  (UINT_MAX == 0xffffffffULL)

    #define ones ones_32
    #define set_from_msb_to_r set_from_msb_to_r_32
    #define log2_floor log2_floor_32
    #define log2_ceil log2_ceil_32
    #define next_pow2 next_pow2_32
    #define round_to_next_pow2 round_to_next_pow2_32
    #define clear_non_msb clear_non_msb_32

#elif  (UINT_MAX == 0xffffffffffffffffULL)

    #define ones ones_64
    #define set_from_msb_to_r set_from_msb_to_r_64
    #define log2_floor log2_floor_64
    #define log2_ceil log2_ceil_64
    #define next_pow2 next_pow2_64
    #define round_to_next_pow2 round_to_next_pow2_64
    #define clear_non_msb clear_non_msb_64

#endif

#endif //__cplusplus

#endif //include guard
