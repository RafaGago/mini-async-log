/*
 * log_packed_protocol.hpp
 *
 *  Created on: Nov 16, 2014
 *      Author: rafgag
 */


#ifndef UFO_LOG_PACKED_PROTOCOL_HPP_
#define UFO_LOG_PACKED_PROTOCOL_HPP_

#include <type_traits>
#include <util/integer.hpp>

// this protocol version (if is ever implemented) will try to squeeze more
// bytes -> reduce heap usage at the expense of some cpu cycles + code
// complexity, the idea is to e.g. encode uint64 values under 256 in one byte.

namespace ufo { namespace proto {

//------------------------------------------------------------------------------
struct header_length_extra_msb
{
    typedef u8 unsigned_type;
    static const uword more_length_msgs_bits = 1;
    static const uword length_bits           = (sizeof (unsigned_type)) * 8 -
                                                more_length_msgs_bits;
    unsigned_type more_length_msgs : more_length_msgs_bits;
    unsigned_type length           : length_bits;
};
//------------------------------------------------------------------------------
struct header
{
    typedef u16 unsigned_type;

    static const uword arity_bits            = 5;                               //arity 0 = empty, arity 1 = string, arity 2 format string + parameter. Could be obliterated if all compilers supported constexpr, as it only is used to check for correctness
    static const uword severity_bits         = 3;
    static const uword overflow_bits         = 1;
    static const uword more_length_msgs_bits = 1;
    static const uword length_bits           = (sizeof (unsigned_type)) * 8 -
                                                more_length_msgs_bits -
                                                arity_bits -
                                                severity_bits -
                                                overflow_bits;

    static const uword length_max = (1 << length_bits) - 1;

    unsigned_type arity            : arity_bits;
    unsigned_type severity         : severity_bits;
    unsigned_type overflow         : overflow_bits;
    unsigned_type more_length_msgs : more_length_msgs_bits;
    unsigned_type length           : length_bits;
};
//------------------------------------------------------------------------------
static_assert (sizeof (header) == sizeof (header::unsigned_type), "");
//------------------------------------------------------------------------------
enum field_class
{
    field_deep_copied = 0,
    field_string      = 1,
    field_numeric     = 2,
};
//------------------------------------------------------------------------------
typedef u8         field_wire_type;
static const uword field_type_bits    = 2;
static const uword type_specific_bits = 6;
//------------------------------------------------------------------------------
struct base_field
{
    field_wire_type type          : field_type_bits;
    field_wire_type type_specific : type_specific_bits;
};
//------------------------------------------------------------------------------
struct deep_copied_field
{
    static const uword is_string_bits = 1;
    static const uword olength_bits   = type_specific_bits - 1;
    static const uword olength_max    = (1 << olength_bits) - 1;

    enum dc_type
    {
        dc_string,
        dc_byte_stream
    };

    field_wire_type type      : field_type_bits; //field_deep_copied
    field_wire_type is_string : is_string_bits;  //dc_string or dc_byte_stream
    field_wire_type olength   : olength_bits;    //olength = "optional length". Length is specified here if it fits, otherwise expect a 0 here with an unsigned numeric afterwards.
};
//------------------------------------------------------------------------------
typedef deep_copied_field dc_field;
//------------------------------------------------------------------------------
struct string_field
{
    static const uword is_literal_bits = 1;

    field_wire_type type       : field_type_bits; //field_string
    field_wire_type is_literal : is_literal_bits;
};
//------------------------------------------------------------------------------
enum numeric_type
{
    num_positive,
    num_negative,
    num_floating,
    num_boolean,
};
static const uword numeric_type_bits = 2;
//------------------------------------------------------------------------------
struct numeric_base
{
    field_wire_type type         : field_type_bits;   //field_numeric
    field_wire_type numeric_type : numeric_type_bits;
    field_wire_type num_specific : type_specific_bits;
};
//------------------------------------------------------------------------------
typedef numeric_base num_base;
//------------------------------------------------------------------------------
struct numeric_integer
{
    static const uword type_bits         = field_type_bits;
    static const uword print_hex_bits    = 1;
    static const uword bytes_bits        = 3; //0 = 1, ... 7 = 8

    field_wire_type type         : field_type_bits;     //field_numeric
    field_wire_type numeric_type : numeric_type_bits;   //num_positive or num_positive
    field_wire_type print_hex    : print_hex_bits;
    field_wire_type bytes        : bytes_bits;
};
//------------------------------------------------------------------------------
typedef numeric_base num_int;
//------------------------------------------------------------------------------
struct numeric_floating_point
{
    static const uword is_double_bits  = 1; //some bits left for formatting, eg best fit, exponential, max resolution

    field_wire_type type           : field_type_bits;     //field_numeric
    field_wire_type numeric_type   : numeric_type_bits;   //num_float
    field_wire_type is_double      : is_double_bits;
};
//------------------------------------------------------------------------------
typedef numeric_floating_point num_fp;
//------------------------------------------------------------------------------
struct numeric_bool
{
    static const uword bool_value_bits = 1;

    field_wire_type type         : field_type_bits;     //field_numeric
    field_wire_type numeric_type : numeric_type_bits;   //num_boolean
    field_wire_type bool_value   : bool_value_bits;
};
//------------------------------------------------------------------------------
typedef numeric_bool num_bool;
//------------------------------------------------------------------------------
union numeric_field
{
    numeric_base           raw;
    numeric_integer        int_;
    numeric_floating_point fp;
    numeric_bool           bool_;
};
//------------------------------------------------------------------------------
union field
{
    base_field        raw;
    deep_copied_field dc;
    numeric_field     num;
    string_field      str;
};
//------------------------------------------------------------------------------
static_assert (sizeof (field) == sizeof (field_wire_type), "");
//------------------------------------------------------------------------------
////------------------------------------------------------------------------------
//static const uword largest_message_bytesize =
//        sizeof (header) + ((1 << header::length_bits) - 1);
//
//static const uword smallest_message_bytesize =
//        sizeof (header) + sizeof (str_literal);
////------------------------------------------------------------------------------
} //namespace proto
////------------------------------------------------------------------------------
//typedef proto::integer<u8>  log_u8;
//typedef proto::integer<u16> log_u16;
//typedef proto::integer<u32> log_u32;
//typedef proto::integer<u64> log_u64;
//typedef proto::integer<i8>  log_i8;
//typedef proto::integer<i16> log_i16;
//typedef proto::integer<i32> log_i32;
//typedef proto::integer<i64> log_i64;
////------------------------------------------------------------------------------
//struct sev
//{
//    enum severity
//    {
//        debug    = 0,
//        trace    = 1,
//        notice   = 2,
//        warning  = 3,
//        error    = 4,
//        critical = 5,
//        off      = 6,
//        invalid  = 7,
//    };
//};
////------------------------------------------------------------------------------

} //namespaces

#endif /* UFO_LOG_LOG_MESSAGE_HPP_ */
