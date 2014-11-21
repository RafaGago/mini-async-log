/*
 * log_message_encoder.hpp
 *
 *  Created on: Nov 12, 2014
 *      Author: rafgag
 */


#ifndef TINY_LOG_LOG_MESSAGE_ENCODER_HPP_
#define TINY_LOG_LOG_MESSAGE_ENCODER_HPP_

#include <cassert>
#include <cstring>
#include <tiny/protocol.hpp>

namespace tiny { namespace proto {

//------------------------------------------------------------------------------
class encoder
{
public:
    //--------------------------------------------------------------------------
    struct null_type {};
    //--------------------------------------------------------------------------
    encoder()
    {
        zero();
    }
    //--------------------------------------------------------------------------
    void init (u8* mem, uword msg_total_size)
    {
        assert (mem);
        assert (msg_total_size >= smallest_message_bytesize);

        m_header.length   = msg_total_size;
        m_header.overflow = 0;

        if (mem &&
            (msg_total_size >= smallest_message_bytesize) &&
            m_header.length == msg_total_size
            )
        {
            m_pos = m_beg = mem;
            m_end = m_pos + msg_total_size;
        }
        else
        {
            assert(
                (m_header.length == msg_total_size) && "header length overflow"
                );
            m_pos = m_beg = m_end = nullptr;
            m_header.length = 0;
        }
    }
    //--------------------------------------------------------------------------
    static uword required_bytes_arity1()
    {
        return smallest_message_bytesize;
    }
    //--------------------------------------------------------------------------
    static uword required_bytes (u8 v)           { return bytes_for (v); }
    static uword required_bytes (u16 v)          { return bytes_for (v); }
    static uword required_bytes (u32 v)          { return bytes_for (v); }
    static uword required_bytes (u64 v)          { return bytes_for (v); }
    static uword required_bytes (i8 v)           { return bytes_for (v); }
    static uword required_bytes (i16 v)          { return bytes_for (v); }
    static uword required_bytes (i32 v)          { return bytes_for (v); }
    static uword required_bytes (i64 v)          { return bytes_for (v); }

    static uword required_bytes (log_u8 v)       { return bytes_for (v.get()); }
    static uword required_bytes (log_u16 v)      { return bytes_for (v.get()); }
    static uword required_bytes (log_u32 v)      { return bytes_for (v.get()); }
    static uword required_bytes (log_u64 v)      { return bytes_for (v.get()); }
    static uword required_bytes (log_i8 v)       { return bytes_for (v.get()); }
    static uword required_bytes (log_i16 v)      { return bytes_for (v.get()); }
    static uword required_bytes (log_i32 v)      { return bytes_for (v.get()); }
    static uword required_bytes (log_i64 v)      { return bytes_for (v.get()); }

    static uword required_bytes (float v)        { return bytes_for (v); }
    static uword required_bytes (double v)       { return bytes_for (v); }
    static uword required_bytes (bool v)         { return bytes_for (v); }
    static uword required_bytes (raw_data v)
    {
        return bytes_for (static_cast<delimited_mem> (v));
    }
    static uword required_bytes (byte_stream v)
    {
        return bytes_for (static_cast<delimited_mem> (v));
    }
    static uword required_bytes (str_literal v)  { return bytes_for (v); }
    static uword required_bytes (const char* v)  { return bytes_for (v); }
    inline static uword required_bytes (null_type)      { return 0; }
    //--------------------------------------------------------------------------
    bool encode_basic (sev::severity sv, uword arity, str_literal fmt)
    {
        assert (can_encode());
        assert (!overflow());
        assert (m_arity == 0);
        assert (arity);
        assert (sv < sev::invalid);

        m_header.arity    = arity;
        m_header.severity = sv;

        if (type_write (m_header) && type_write (fmt))
        {
            m_arity = 1;
            return true;
        }
        assert (false && "bug!");
        return false;
    }
    //--------------------------------------------------------------------------
    bool encode (u8 v)  { return write (v, field_u8); }
    bool encode (u16 v) { return write (v, field_u16); }
    bool encode (u32 v) { return write (v, field_u32); }
    bool encode (u64 v) { return write (v, field_u64); }
    bool encode (i8 v)  { return write (v, field_u8); }
    bool encode (i16 v) { return write (v, field_u16); }
    bool encode (i32 v) { return write (v, field_u32); }
    bool encode (i64 v) { return write (v, field_u64); }

    bool encode (log_u8 v)  { return write (v.get(), field_u8, v.flags()); }
    bool encode (log_u16 v) { return write (v.get(), field_u16, v.flags()); }
    bool encode (log_u32 v) { return write (v.get(), field_u32, v.flags()); }
    bool encode (log_u64 v) { return write (v.get(), field_u64, v.flags()); }
    bool encode (log_i8 v)  { return write (v.get(), field_u8, v.flags()); }
    bool encode (log_i16 v) { return write (v.get(), field_u16, v.flags()); }
    bool encode (log_i32 v) { return write (v.get(), field_u32, v.flags()); }
    bool encode (log_i64 v) { return write (v.get(), field_u64, v.flags()); }

    bool encode (float v)       { return write (v, field_float); }
    bool encode (double v)      { return write (v, field_double); }
    bool encode (bool v)        { return write (v, field_bool); }
    bool encode (raw_data v)
    {
        return write (static_cast<delimited_mem> (v), field_raw);
    }
    bool encode (byte_stream v)
    {
        return write (static_cast<delimited_mem> (v), field_byte_stream);
    }
    bool encode (str_literal v) { return write (v, field_str_literal); }
    bool encode (const char* v)
    {
        return write (v, field_whole_program_duration_c_str);
    }
    //--------------------------------------------------------------------------
    inline bool encode (null_type) { return true; };
    //--------------------------------------------------------------------------
    bool can_encode() const
    {
        return m_beg != nullptr;
    }
    //--------------------------------------------------------------------------
    uint8* get_result()
    {
        uint8* ret = m_beg;
        if (m_arity == m_header.arity)
        {
            assert (!overflow());
            assert (m_header.length == m_pos - m_beg);
            assert (m_header.arity  == m_arity);
        }
        else
        {
            assert (overflow());
            m_header.arity  = m_arity;
            m_header.length = m_pos - m_beg;
            m_pos           = m_beg;
            type_write (m_header);
        }
        zero();
        return ret;
    }
    //--------------------------------------------------------------------------

private:
    //--------------------------------------------------------------------------
    void zero()
    {
        m_beg = m_pos = m_end = nullptr;
        m_header.arity    = 0;
        m_header.length   = 0;
        m_header.overflow = 0;
        m_header.severity = 0;
        m_arity           = 0;
    }
    //--------------------------------------------------------------------------
    bool overflow() const
    {
        return m_header.overflow != 0;
    }
    //--------------------------------------------------------------------------
    template <class T>
    static uword bytes_for (T)
    {
        return sizeof (field) + sizeof (T);
    }
    //--------------------------------------------------------------------------
    static uword bytes_for (delimited_mem r)
    {
        return sizeof (field) +
               sizeof (variable_deep_copy_field) +
               (r.mem ? r.size : 0);
    }
    //--------------------------------------------------------------------------
    static uword bytes_for (bool b)
    {
        return sizeof (field);
    }
    //--------------------------------------------------------------------------
    bool write_overflow()
    {
        m_header.overflow = 1;
        return false;
    }
    //--------------------------------------------------------------------------
    template <class T>
    bool write (T v, field::unsigned_type type, field::unsigned_type flags = 0)
    {
        assert (m_pos && m_header.length && m_header.arity);
        field f;
        f.flags = flags;
        f.type  = type;
        auto sz = required_bytes (v);
        if ((m_pos + sz) <= m_end)
        {
            std::memcpy (m_pos, &f, sizeof f);
            m_pos += sizeof f;
            std::memcpy (m_pos, &v, sizeof v);
            m_pos += sizeof v;
            ++m_arity;
            return true;
        }
        return write_overflow();
    }
    //--------------------------------------------------------------------------
    bool write (delimited_mem v, field::unsigned_type type)
    {
        assert (m_pos && m_header.length && m_header.arity);
        field f;
        f.flags   = 0;
        f.type    = type;
        variable_deep_copy_field dc;
        dc.length = v.size;
        assert ((dc.length == v.size) && "deep copy counter overflow");
        auto sz = bytes_for (v);
        if ((m_pos + sz) <= m_end)
        {
            std::memcpy (m_pos, &f, sizeof f);
            m_pos += sizeof f;
            std::memcpy (m_pos, &dc, sizeof dc);
            m_pos += sizeof dc;
            if (v.mem && dc.length)
            {
                std::memcpy (m_pos, v.mem, dc.length);
                m_pos += dc.length;
            }
            ++m_arity;
            return true;
        }
        return write_overflow();
    }
    //--------------------------------------------------------------------------
    bool write (bool v, field::unsigned_type type)
    {
        assert (m_pos && m_header.length && m_header.arity);
        field f;
        f.flags = v ? 1 : 0;
        f.type  = type;
        auto sz = bytes_for (v);
        if ((m_pos + sz) <= m_end)
        {
            std::memcpy (m_pos, &f, sizeof f);
            m_pos += sizeof f;
            ++m_arity;
            return true;
        }
        return write_overflow();
    }
    //--------------------------------------------------------------------------
    template <class T>
    bool type_write (T val)
    {
        assert (m_pos && m_header.length && m_header.arity);
        auto sz  = sizeof val;
        if (m_pos + sz <= m_end)
        {
            std::memcpy (m_pos, &val, sz);
            m_pos += sz;
            return true;
        }
        else
        {
            return false;
        }
    }
    //--------------------------------------------------------------------------
    u8*    m_pos;
    u8*    m_beg;
    u8*    m_end;
    header m_header;
    uword  m_arity;

}; //class log_message_decoder
//------------------------------------------------------------------------------

}} //namespaces
#endif /* TINY_LOG_LOG_MESSAGE_ENCODER_HPP_ */
