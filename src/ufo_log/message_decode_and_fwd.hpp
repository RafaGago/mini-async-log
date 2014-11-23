/*
 * log_message_decoder.hpp
 *
 *  Created on: Nov 12, 2014
 *      Author: rafgag
 */


#ifndef UFO_LOG_LOG_MESSAGE_DECODER_HPP_
#define UFO_LOG_LOG_MESSAGE_DECODER_HPP_

#include <cassert>
#include <cstring>
#include <ufo_log/protocol.hpp>
#include <ufo_log/output.hpp>
#include <ufo_log/byte_stream_convert.hpp>

namespace ufo { namespace proto {

//------------------------------------------------------------------------------
class decode_and_fwd
{
public:
    //--------------------------------------------------------------------------
    decode_and_fwd()
    {
        zero();
        prints_timestamp = true;
        prints_severity  = true;
    }
    //--------------------------------------------------------------------------
    void new_entry (const u8* mem)
    {
        zero();
        assert (mem);
        if (mem)
        {
            m_pos    = mem;
            m_header = type_extract<header>();
            m_end    = m_pos + m_header.length;
        }
    }
    //--------------------------------------------------------------------------
    bool has_content()
    {
        assert ((m_header.length != 0 && m_header.arity != 0) ||
                (m_header.length == 0 && m_header.arity == 0)
                );
        return m_header.length != 0;
    }
    //--------------------------------------------------------------------------
    void decode_and_fwd_entry (output& o, u64 tstamp)
    {
        start_decoding_common (o, (sev::severity) m_header.severity, tstamp);

        str_literal fmt;
        if (fits<str_literal>())
        {
            fmt = type_extract<str_literal>();
        }
        else
        {
            assert (false && "bug !");
            static const char bug_str[] = "[logger err]->bug!\n";
            o.write (bug_str, sizeof bug_str - 1);
            return;
        }

        uword remaining = m_header.arity - 1;
        auto curr       = (const char*) fmt.mem;
        auto end        = curr + fmt.size;

        while (remaining && fits<field>())
        {
            if (auto next = find_next_placeholder (curr, end))
            {
                o.write (curr, next - curr - fmt_token_char_count);
                curr         = next;
                auto f       = type_extract<field>();
                bool success = content_extract (o, f);
                if (success)
                {
                    --remaining;
                }
                else
                {
                    static const char corrupted[] =
                            "[logger err]->corrupted entry\n";
                    o.write (corrupted, sizeof corrupted - 1);
                    return;
                }
            }
            else
            {
                static const char defect_params[] =
                    "[logger err]->missing placeholder(s) in format string\n";      //can be detected at compile time if compiler supports constexpr
                o.write (curr, end - curr);
                o.write (defect_params, sizeof defect_params - 1);
                return;
            }
        }

        auto next = find_next_placeholder (curr, end);
        if (next == nullptr)                                                    //format remainder or format with no parameters
        {
            o.write (curr, end - curr);
            curr         = next;
            if (!m_header.overflow)
            {
                static const char newline = '\n';
                o.write (&newline, sizeof newline);
            }
            else
            {
                static const char overflow_str[] =
                        "[logger err]->entry overflow\n";
                o.write (overflow_str, sizeof overflow_str - 1);
            }
        }
        else
        {
            static const char excess_params[] =
                "[logger err]->excess of placeholder(s) in format string\n";  //can be detected at compile time if compiler supports constexpr
            o.write (curr, next - curr - fmt_token_char_count);
            o.write (excess_params, sizeof excess_params - 1);
        }
    }
    //--------------------------------------------------------------------------
    void fwd_alloc_fault_entry (output& o, u64 t, uword count)
    {
        start_decoding_common (o, sev::error, t);
        u64 c64       = count;
        output_num (o, c64, "%llu");
        char str[]    = " entries lost by queue allocation faults\n";
        o.write (str, sizeof str - 1);
    }
    //--------------------------------------------------------------------------
    bool prints_severity;
    //--------------------------------------------------------------------------
    bool prints_timestamp;
    //--------------------------------------------------------------------------
private:
    //--------------------------------------------------------------------------
    static const uword fmt_token_char_count = 2;
    //--------------------------------------------------------------------------
    void zero()
    {
        m_pos = m_end = nullptr;
        std::memset (&m_header, 0, sizeof m_header);
    }
    //--------------------------------------------------------------------------
    void start_decoding_common (output& o, sev::severity s, u64 t)
    {
        o.next_writes_severity (s);
        if (prints_timestamp) { write_timestamp (o, t); }
        if (prints_severity)  { write_severity (o, s); }
    }
    //--------------------------------------------------------------------------
    template <class T>
    static bool output_num (output& o, T val, const char* fmt)
    {
        char buff[64];                                                          //just to skip thinking, never is going to be that big.
        uword adv = std::snprintf (buff, sizeof buff, fmt, val);
        if (adv > 0)
        {
            o.write (buff, adv);
            return true;
        }
        assert (false && "bug!");
        return false;
    }
    //--------------------------------------------------------------------------
    static void write_severity (output& o, sev::severity s)
    {
        switch (s)
        {
        case sev::debug   :
        {
            char str[] = "[dbg-] ";
            o.write (str, sizeof str - 1);
            break;
        }
        case sev::trace   :
        {
            char str[] = "[trac] ";
            o.write (str, sizeof str - 1);
            break;
        }
        case sev::notice  :
        {
            char str[] = "[note] ";
            o.write (str, sizeof str - 1);
            break;
        }
        case sev::warning :
        {
            char str[] = "[warn] ";
            o.write (str, sizeof str - 1);
            break;
        }
        case sev::error   :
        {
            char str[] = "[err-] ";
            o.write (str, sizeof str - 1);
            break;
        }
        case sev::critical:
        {
            char str[] = "[crit] ";
            o.write (str, sizeof str - 1);
            break;
        }
        default      :
            break;
        }
    }
    //--------------------------------------------------------------------------
    static void write_timestamp (output& o, u64 t)
    {
        output_num (o, t, "[%020llu] ");
    }
    //--------------------------------------------------------------------------
    const char* find_next_placeholder (const char* curr, const char* end)
    {
        assert (curr && end && curr <= end);
        while (curr && (curr < (end - 1)))
        {
            curr = (const char*) std::memchr (curr, '{', end - curr);
            if (curr && *++curr == '}')
            {
                return ++curr;
            }
        }
        return nullptr;
    }
    //--------------------------------------------------------------------------
    bool pending_data() const
    {
        return m_pos && (m_pos < m_end);
    }
    //--------------------------------------------------------------------------
    bool fits (uword bytes) const
    {
        return m_pos + bytes <= m_end;
    }
    //--------------------------------------------------------------------------
    template <class T>
    bool fits() const
    {
        return fits (sizeof (T));
    }
    //--------------------------------------------------------------------------
    template <class T>
    bool extract_write_num (output& o, const char* fmt)
    {
        if (fits<T>())
        {
            T val = type_extract<T>();
            return output_num (o, val, fmt);
        }
        return false;
    }
    //--------------------------------------------------------------------------
    bool content_extract (output& o, field f)
    {
        switch (f.type)
        {
        case field_raw            :
        {
            if (fits<variable_deep_copy_field>())
            {
                variable_deep_copy_field r =
                        type_extract<variable_deep_copy_field>();
                if (fits (r.length))
                {
                    o.write ((const char*) m_pos, r.length);
                    m_pos += r.length;
                    return true;
                }
            }
            return false;
        }
        case field_u8             :
        {
            return extract_write_num<u8>(
                    o, (log_u8::has_hex (f.flags)) ? "0x%02x" : "%u"
                    );
        }
        case field_u16            :
        {
            return extract_write_num<u16>(
                    o, (log_u16::has_hex (f.flags)) ? "0x%04hx" : "%hu"
                    );
        }
        case field_u32            :
        {
            return extract_write_num<u32>(
                    o, (log_u32::has_hex (f.flags)) ? "0x%08lx" : "%lu"
                    );
        }
        case field_u64            :
        {
            return extract_write_num<u64>(
                    o, (log_u64::has_hex (f.flags)) ? "0x%016llx" : "%llu"
                    );
        }
        case field_i8             :
        {
            return extract_write_num<i8>(
                    o, (log_i8::has_hex (f.flags)) ? "0x%02x" : "%d"
                    );
        }
        case field_i16            :
        {
            return extract_write_num<i16>(
                    o, (log_i16::has_hex (f.flags)) ? "0x%04hx" : "%hd"
                    );
        }
        case field_i32            :
        {
            return extract_write_num<i32>(
                    o, (log_i32::has_hex (f.flags)) ? "0x%08lx" : "%ld"
                    );
        }
        case field_i64            :
        {
            return extract_write_num<i64>(
                    o, (log_i64::has_hex (f.flags)) ? "0x%16llx" : "%lld"
                    );
        }
        case field_float          :
        {
            return extract_write_num<float> (o, "%g");
        }
        case field_double         :
        {
            return extract_write_num<double> (o, "%g");
        }
        case field_bool           :
        {
            static const char true_val[]  = "true";
            static const char false_val[] = "false";
            if (f.flags != 0) { o.write (true_val, sizeof true_val - 1); }
            else              { o.write (false_val, sizeof false_val - 1); }
            return true;
        }
        case field_str_literal    :
        {
            if (fits<str_literal>())
            {
                str_literal s = type_extract<str_literal>();
                o.write ((char*) s.mem, s.size);
                return true;
            }
            return false;
        }
        case field_whole_program_duration_c_str   :
        {
            typedef const char* char_ptr_type;
            if (fits<char_ptr_type>())
            {
                char_ptr_type s = type_extract<char_ptr_type>();
                o.write (s);
                return true;
            }
            return true;
        }
        case field_byte_stream            :
        {
            if (fits<variable_deep_copy_field>())
            {
                variable_deep_copy_field r =
                        type_extract<variable_deep_copy_field>();
                if (fits (r.length))
                {
                    byte_stream_convert::execute (o, m_pos, r.length);
                    m_pos += r.length;
                    return true;
                }
            }
            return false;
        }
        default:
            return false;
        }
    }
    //--------------------------------------------------------------------------
    template <class T>
    T type_extract()
    {
        T val;
        auto end = m_pos + sizeof val;
        for (auto out = (u8*) &val; m_pos < end; ++m_pos, ++out)
        {
            *out = *m_pos;
        }
        return val;
    }
    //--------------------------------------------------------------------------
    const u8* m_pos;
    const u8* m_end;
    header    m_header;
}; //class log_message_decoder
//------------------------------------------------------------------------------
}} //namespaces

#endif /* UFO_LOG_LOG_MESSAGE_DECODER_HPP_ */
