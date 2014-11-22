/*
 * log_protocol.hpp
 *
 *  Created on: Nov 12, 2014
 *      Author: rafgag
 */


#ifndef TINY_LOG_PROTOCOL_HPP_
#define TINY_LOG_PROTOCOL_HPP_

#include <type_traits>
#include <tiny/util/integer.hpp>

namespace tiny { namespace proto {

//------------------------------------------------------------------------------
struct field
{
    typedef u8 unsigned_type;

    static const uword type_bits  = 5;
    static const uword flags_bits = (sizeof (unsigned_type) * 8) - type_bits;

    unsigned_type type  : type_bits;
    unsigned_type flags : flags_bits;
};
//------------------------------------------------------------------------------
static_assert (sizeof (field) == sizeof (field::unsigned_type), "");
//------------------------------------------------------------------------------
struct header
{
    typedef u32 unsigned_type;

    static const uword arity_bits    = 5;                                       //arity 0 = empty, arity 1 = string, arity 2 format string + parameter
    static const uword severity_bits = 3;
    static const uword overflow_bits = 1;
    static const uword length_bits   =
            (sizeof (unsigned_type)) * 8 -
            arity_bits -
            severity_bits -
            overflow_bits;

    unsigned_type arity    : arity_bits;
    unsigned_type severity : severity_bits;
    unsigned_type overflow : overflow_bits;
    unsigned_type length   : length_bits;
};
//------------------------------------------------------------------------------
static_assert (sizeof (header) == sizeof (header::unsigned_type), "");
//------------------------------------------------------------------------------
struct delimited_mem
{
    const void* mem;
    uword       size;
};
////------------------------------------------------------------------------------
//bool deconstruct()
//{
//    u32 v;
//
//}
//------------------------------------------------------------------------------
struct str_literal : public delimited_mem
{
    str_literal()
    {
        mem  = nullptr;
        size = 0;
    }

    template <uword N>
    str_literal (const char (&literal)[N])
    {
        static_assert( N >= 1, "not a string literal");
        mem  = (const void*) literal;
        size = N - 1;
    }

    str_literal (const str_literal& other)
    {
        *this = other;
    }

    str_literal &operator= (const str_literal& other)
    {
        mem  = other.mem;
        size = other.size;
        return *this;
    }
};
//------------------------------------------------------------------------------
struct raw_data : public delimited_mem {};
//------------------------------------------------------------------------------
struct byte_stream : public delimited_mem {};
//------------------------------------------------------------------------------
struct variable_deep_copy_field
{
    typedef header::unsigned_type unsigned_type;
    unsigned_type length : header::length_bits;
};
//------------------------------------------------------------------------------
enum fields
{
    field_raw                          = 0,
    field_u8                           = 1,
    field_u16                          = 2,
    field_u32                          = 3,
    field_u64                          = 4,
    field_i8                           = 5,
    field_i16                          = 6,
    field_i32                          = 7,
    field_i64                          = 8,
    field_float                        = 9,
    field_double                       = 10,
    field_bool                         = 11,
    field_str_literal                  = 12,
    field_whole_program_duration_c_str = 13,
    field_byte_stream                  = 14
};
//------------------------------------------------------------------------------
template <class T>
class integer
{
public:

    static_assert(
        std::is_integral<T>::value &&
        !std::is_same<typename std::decay<T>::type, bool>::value,
        "invalid type");

    integer (T v, bool hex = false)
    {
        m_biggest = (u64) v;
        m_flags   = (hex) ? 1 : 0;
    }

    integer (const integer<T>& other)
    {
        *this = other;
    }

    integer<T>& operator= (const integer<T>& other)
    {
        m_biggest  = other.m_biggest;
        m_flags    = other.m_flags;
        return *this;
    }

    T get() const
    {
        return (T) m_biggest;
    }

    field::unsigned_type flags()
    {
        return m_flags;
    }

    static bool has_hex (field::unsigned_type flags)
    {
        return (flags & 1) ? true : false;
    }

private:
    u64                  m_biggest;
    field::unsigned_type m_flags;
};
//------------------------------------------------------------------------------
static const uword largest_message_bytesize =
        sizeof (header) + ((1 << header::length_bits) - 1);

static const uword smallest_message_bytesize =
        sizeof (header) + sizeof (str_literal);
//------------------------------------------------------------------------------
} //namespace proto
//------------------------------------------------------------------------------
typedef proto::integer<u8>  log_u8;
typedef proto::integer<u16> log_u16;
typedef proto::integer<u32> log_u32;
typedef proto::integer<u64> log_u64;
typedef proto::integer<i8>  log_i8;
typedef proto::integer<i16> log_i16;
typedef proto::integer<i32> log_i32;
typedef proto::integer<i64> log_i64;
//------------------------------------------------------------------------------
struct sev
{
    enum severity
    {
        debug    = 0,
        trace    = 1,
        notice   = 2,
        warning  = 3,
        error    = 4,
        critical = 5,
        off      = 6,
        invalid  = 7,
    };
};
//------------------------------------------------------------------------------

} //namespace tiny

#endif /* TINY_LOG_LOG_MESSAGE_HPP_ */
