/*
 * log_byte_stream_convert.hpp
 *
 *  Created on: Nov 15, 2014
 *      Author: rafgag
 */


#ifndef UFO_LOG_LOG_BYTE_STREAM_CONVERT_HPP_
#define UFO_LOG_LOG_BYTE_STREAM_CONVERT_HPP_

#include <ufo_log/output.hpp>

// disclaimer: this is an overkill when dealing with files, it's a copy-paste
//             of some code I had lying around. Then I felt bad, I simplified
//             it but everything remains the same: it still is a miserable
//             overkill.

namespace ufo {
//------------------------------------------------------------------------------
namespace detail {
#if 0
    static const char hex_lut[] = "000102030405060708090a0b0c0d0e0f"
                                  "101112131415161718191a1b1c1d1e1f"
                                  "202122232425262728292a2b2c2d2e2f"
                                  "303132333435363738393a3b3c3d3e3f"
                                  "404142434445464748494a4b4c4d4e4f"
                                  "505152535455565758595a5b5c5d5e5f"
                                  "606162636465666768696a6b6c6d6e6f"
                                  "707172737475767778797a7b7c7d7e7f"
                                  "808182838485868788898a8b8c8d8e8f"
                                  "909192939495969798999a9b9c9d9e9f"
                                  "a0a1a2a3a4a5a6a7a8a9aaabacadaeaf"
                                  "b0b1b2b3b4b5b6b7b8b9babbbcbdbebf"
                                  "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf"
                                  "d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"
                                  "e0e1e2e3e4e5e6e7e8e9eaebecedeeef"
                                  "f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff";
    template<uword remaining>
    struct hex_unroll
    {
        inline static const u8* convert (char* dst, const u8* src)
        {
            *dst++ = hex_lut[((*src_buff) * 2)];
            *dst++ = hex_lut[((*src_buff) * 2) + 1];
            return hex_unroll<remaining - 1>::convert (dst, ++src);
        }
    };

    template<>
    struct hex_unroll<0>
    {
        inline static const u8* convert (char* dst, const u8* src)
        {
            return src;
        }
    };
#endif

    static const char hex_lut[] = "0123456789abcdef";
    template<uword remaining>
    struct hex_unroll
    {
        inline static const u8* convert (char* dst, const u8* src)
        {
            *dst++ = hex_lut[(*src) >> 4];
            *dst++ = hex_lut[(*src) &  15];
            return hex_unroll<remaining - 1>::convert (dst, ++src);
        }
    };

    template<>
    struct hex_unroll<0>
    {
        inline static const u8* convert (char* dst, const u8* src)
        {
            return src;
        }
    };
//------------------------------------------------------------------------------
} //namespace detail
//------------------------------------------------------------------------------
class byte_stream_convert
{
    public:

    static inline void execute (output& o, const u8* mem, uword sz)
    {

        static const uword unroll_bits = 4;
        static const uword unroll_mask = (1 << unroll_bits) - 1;
        static const uword unroll_size = 1 << unroll_bits;

        assert (mem);

        char buff[unroll_size * 2];

        uword end = sz >> unroll_bits;
        for (uword i = 0; i < end; ++i)
        {
            mem = detail::hex_unroll<unroll_size>::convert (buff, mem);
            o.write (buff, sizeof buff);
        }

        end = sz & unroll_mask;
        for (uword i = 0; i < end; ++i)
        {
            mem = detail::hex_unroll<1>::convert (buff, mem);
            o.write (buff, 2);
        }
    }
}; //class byte_stream
//------------------------------------------------------------------------------
} //namespace

#endif /* UFO_LOG_LOG_BYTE_STREAM_CONVERT_HPP_ */
