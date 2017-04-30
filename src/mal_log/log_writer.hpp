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

#ifndef MAL_LOG_WRITER_HPP_
#define MAL_LOG_WRITER_HPP_

#include <mal_log/util/mem_printf.hpp>
#include <mal_log/serialization/printf_modifiers.hpp>
#include <mal_log/serialization/byte_stream_convert.hpp>
#include <mal_log/serialization/importer.hpp>
#include <mal_log/mal_private.hpp>
#include <mal_log/timestamp.hpp>
#include <mal_log/output.hpp>
#include <mal_log/format_tokens.hpp>
#include <mal_log/async_to_sync.hpp>

namespace mal {

//------------------------------------------------------------------------------
class log_writer : private ser::importer
{
public:
    //--------------------------------------------------------------------------
    log_writer()
    {
        m_timestamp_base = 0;
        m_fmt            = nullptr;
        m_fmt_modif      = 0;
        prints_severity  = prints_timestamp = true;
        m_sync           = nullptr;
    }
    //--------------------------------------------------------------------------
    void set_synchronizer (async_to_sync& sync)
    {
        m_sync = &sync;
    }
    //--------------------------------------------------------------------------
    void set_timestamp_base (u64 base)
    {
        m_timestamp_base = base;
    }
    //--------------------------------------------------------------------------
    bool decode_and_write (output& o, const u8* msg)
    {
        assert (msg);

        init (msg);

        ser::header_data h;
        do_import (h);

        if (h.sync != nullptr) { m_sync->notify (*h.sync); }

        set_next_msg_fmt_string (h.fmt);
        o.entry_begin (h.severity);

        if (prints_timestamp) {
            write_timestamp(
                o,
                h.has_tstamp ?
                    h.tstamp : (get_ns_timestamp() - m_timestamp_base)
                );
        }
        if (prints_severity) { write_severity (o, h.severity); }

        uword params   = h.arity;
        bool fmt_param = find_param_in_fmt_str (o, params > 0);

        while (params > 0 && fmt_param) {
            consume_next (o, true);
            --params;
            fmt_param = find_param_in_fmt_str (o, params > 0);
        }

#ifdef MAL_COMPILE_TIME_FMT_CHECK
        assert (params == 0 && !fmt_param);
#else
        if (params) {
            do {
                consume_next (o, false);
                --params;
            }
            while (params);
            static const char excess_params[] =
                "\n[logger err]->too many parameters in previous log entry. "
                 "fmt string was: ";
            o.write (excess_params, sizeof excess_params - 1);
            o.write (h.fmt);
            assert (false && "too many parameters, see log output for details");
        }
        if (fmt_param) {
            do {
                fmt_param = find_param_in_fmt_str (o, params > 0);
            }
            while (fmt_param);
            static const char excess_placeholders[] =
                "\n[logger err]->too many placeholders in previous log entry. "
                  "fmt string was: ";
            o.write (excess_placeholders, sizeof excess_placeholders - 1);
            o.write (h.fmt);
            assert(
                false && "too many placeholders, see log output for details"
                );
        }
#endif
        o.entry_end();
        if (h.severity >= sev::critical) {
            o.flush();
        }
        return true;
    }
    //--------------------------------------------------------------------------
    bool prints_severity;
    //--------------------------------------------------------------------------
    bool prints_timestamp;
    //--------------------------------------------------------------------------
private:
    //--------------------------------------------------------------------------
    void consume_next (output& o, bool has_placeholder)
    {
        using namespace ser;
        ser::decoding_field d;
        import_type (d);
        if (d.gen.fclass == mal_numeric) {
            if (d.gen.nclass == mal_integral) {
                output_integral (o, d.num_int, has_placeholder);
            }
            else {
                assert (d.gen.nclass == mal_non_integral);
                output_non_integral (o, d.nom_no_int, has_placeholder);
            }
        }
        else {
            assert (d.gen.fclass == mal_non_numeric);
            output_non_numeric (o, d.no_num, has_placeholder);
        }
    }
    //--------------------------------------------------------------------------
    void output_integral(
            output& o, ser::integral_field f, bool has_placeholder
            )
    {
        using namespace ser;
        uword type = f.original_type;
        type      += ((uword) f.is_negative) * 4;

        switch (type) {
        case 0: return output_char<u8_modif> (o, f, has_placeholder);
        case 1: return output_int_type<u16, u16_modif> (o, f, has_placeholder);
        case 2: return output_int_type<u32, u32_modif> (o, f, has_placeholder);
        case 3: return output_int_type<u64, u64_modif> (o, f, has_placeholder);
        case 4: return output_char<i8_modif> (o, f, has_placeholder);
        case 5: return output_int_type<i16, i16_modif> (o, f, has_placeholder);
        case 6: return output_int_type<i32, i32_modif> (o, f, has_placeholder);
        case 7: return output_int_type<i64, i64_modif> (o, f, has_placeholder);
        default: return;
        }
    }
    //--------------------------------------------------------------------------
    void output_non_integral(
            output& o, ser::non_integral_field f, bool has_placeholder
            )
    {
        using namespace ser;
        switch (f.niclass) {
        case mal_double:
            return output_floating_type<double, u64, double_modif>(
                    o, f, has_placeholder
                    );
        case mal_float :
            return output_floating_type<float, u32, float_modif>(
                                            o, f, has_placeholder
                                            );
        case mal_bool: {
            if (!has_placeholder) { return; }

            static const char true_val[]  = "true";
            static const char false_val[] = "false";
            if (f.bool_val != 0) { o.write (true_val, sizeof true_val - 1); }
            else                 { o.write (false_val, sizeof false_val - 1); }
            break;
        }
        default:
            assert (false && "bug");
            break;
        }
    }
    //--------------------------------------------------------------------------
    void output_non_numeric(
            output& o, ser::non_numeric_field f, bool has_placeholder
            )
    {
        using namespace ser;
        switch (f.nnclass) {
        case mal_c_str: {
            literal_wrapper l;
            do_import (l, f);
            if (has_placeholder && l.lit) {
                o.write (l.lit);
            }
            else if (!l.lit) {
                static const char lit_null_err[] =
                    "([logger err]->nullptr literal, ignored)";
                o.write (lit_null_err, sizeof lit_null_err - 1);
                assert (false && "null literal");
            }
            break;
        }
        case mal_ptr: {
            static_assert (sizeof (uword) == 4 || sizeof (uword) == 8, "");
            const char* fmt = (sizeof (uword) == 4) ?
                                    u32_modif::hex : u64_modif::hex;
            ptr_wrapper p;
            do_import (p, f);
            uword v = (uword) p.ptr;

            if (has_placeholder) {
                output_num (o, v, fmt);
            }
            break;
        }
        case mal_deep_copied_str : {
            deep_copy_string str;
            do_import (str, f);
            if (has_placeholder) {
                o.write (str.mem, str.size);
            }
            break;
        }
        case mal_deep_copied_mem : {
            deep_copy_bytes bytes;
            do_import (bytes, f);
            if (has_placeholder) {
                byte_stream_convert::execute(
                        o, (const u8*) bytes.mem, bytes.size
                        );
            }
            break;
        }
        default:
            assert (false && "bug");
            break;
        }
    }
    //--------------------------------------------------------------------------
    template <class fmt_struct>
    void output_char(
            output& o, ser::integral_field f, bool has_placeholder
            )
    {
        using namespace ser;
        i8 v;
        do_import (v, f);
        const char* fmt;
        switch (m_fmt_modif) {
        case 0:
            fmt = fmt_struct::norm;
            break;
        case fmt::hex:
            fmt = fmt_struct::hex;
            break;
        case fmt::full_width:
            fmt = fmt_struct::fwidth;
            break;
        case fmt::ascii:
            fmt = fmt_struct::ascii;
            break;
        default:
            if (has_placeholder) {
                write_invalid_modifier (o);
            }
            fmt = i8_modif::norm;
            break;
        }
        if (has_placeholder) {
            output_num (o, v, fmt);
        }
    }
    //--------------------------------------------------------------------------
    template <class T, class fmt_struct>
    void output_int_type(
            output& o, ser::integral_field f, bool has_placeholder
            )
    {
        T v;
        do_import (v, f);
        const char* fmt;
        switch (m_fmt_modif) {
        case 0:
            fmt = fmt_struct::norm;
            break;
        case fmt::hex:
            fmt = fmt_struct::hex;
            break;
        case fmt::full_width:
            fmt = fmt_struct::fwidth;
            break;
        default:
            if (has_placeholder) {
                write_invalid_modifier (o);
            }
            fmt = fmt_struct::norm;
            break;
        }
        if (has_placeholder) {
            output_num (o, v, fmt);
        }
    }
    //--------------------------------------------------------------------------
    template <class T, class H, class fmt_struct>
    void output_floating_type(
            output& o, ser::non_integral_field f, bool has_placeholder)
    {
        union hex_hack {
            T floating;
            H hex;
        };
        hex_hack v;
        do_import (v.floating, f);
        const char* fmt;
        switch (m_fmt_modif) {
        case 0:
            fmt = fmt_struct::norm;
            break;
        case fmt::hex:
            fmt = fmt_struct::hex;
            break;
        case fmt::scientific:
            fmt = fmt_struct::sci;
            break;
        default:
            if (has_placeholder) {
                write_invalid_modifier (o);
            }
            fmt = fmt_struct::norm;
            break;
        }
        if (has_placeholder) {
            if (fmt != fmt_struct::hex) {
                output_num (o, v.floating, fmt);
            }
            else {
                output_num (o, v.hex, fmt);
            }
        }
    }
    //--------------------------------------------------------------------------
    void set_next_msg_fmt_string (const char* fmt)
    {
        m_fmt = fmt;
    }
    //--------------------------------------------------------------------------
    bool find_param_in_fmt_str (output& o, bool remaining_parameters = true)
    {
        assert (m_fmt);
        static const char param_error[] = "{a parameter was expected here}";

        auto fmt_prev = m_fmt;
        while (m_fmt != nullptr) {
            auto found = std::strchr (m_fmt, fmt::placeholder_open);
            if (found && found[1] != 0) {
                m_fmt       = found + 1;
                m_fmt_modif = *m_fmt;
            }
            else {
                o.write (fmt_prev);
                m_fmt_modif = 0;
                return false;
            }
            if (*m_fmt == fmt::placeholder_close) {
                m_fmt_modif = 0;
                ++m_fmt;
                o.write (fmt_prev, found - fmt_prev);
            }
            else if (*(m_fmt + 1) == fmt::placeholder_close) {
                m_fmt += 2;
                o.write (fmt_prev, found - fmt_prev);
            }
            else { continue; }

            if (!remaining_parameters) {
                o.write (param_error, sizeof param_error - 1);
            }
            return true;
        }
        m_fmt_modif = 0;
        return false;
    }
    //--------------------------------------------------------------------------
    template <class T>
    static bool output_num (output& o, T val, const char* fmt)
    {
        char buff[64];                                                          //just to skip thinking, never is going to be that big.
        uword adv = mem_printf (buff, sizeof buff, fmt, val);
        if (adv > 0) {
            o.write (buff, adv);
            return true;
        }
        assert (false && "bug!");
        return false;
    }
    //--------------------------------------------------------------------------
    static void write_severity (output& o, sev::severity s)
    {
        switch (s) {
        case sev::debug: {
            char str[] = "[dbug] ";
            o.write (str, sizeof str - 1);
            break;
        }
        case sev::trace: {
            char str[] = "[trac] ";
            o.write (str, sizeof str - 1);
            break;
        }
        case sev::notice: {
            char str[] = "[note] ";
            o.write (str, sizeof str - 1);
            break;
        }
        case sev::warning: {
            char str[] = "[warn] ";
            o.write (str, sizeof str - 1);
            break;
        }
        case sev::error: {
            char str[] = "[err_] ";
            o.write (str, sizeof str - 1);
            break;
        }
        case sev::critical: {
            char str[] = "[crit] ";
            o.write (str, sizeof str - 1);
            break;
        }
        default      :
            break;
        }
    }
    //--------------------------------------------------------------------------
    static void write_invalid_modifier (output& o)
    {
        static const char invalid_modif_str[] =
                "([logger err]->invalid modifier for next parameter, ignored) ";
        o.write (invalid_modif_str, sizeof invalid_modif_str - 1);
        assert (false && "invalid modifier");
    }
    //--------------------------------------------------------------------------
    static void write_timestamp (output& o, u64 t)
    {
        const u64 ns_sec = 1000000000;
        u64 s = t / ns_sec;
        output_num (o, s, "%011llu.");
        t -= (s * ns_sec);
        output_num (o, t, "%09llu ");
    }
    //--------------------------------------------------------------------------
    u64            m_timestamp_base;
    const char*    m_fmt;
    async_to_sync* m_sync;
    char           m_fmt_modif;
};
//------------------------------------------------------------------------------
} //namespaces
//------------------------------------------------------------------------------

#endif /* MAL_LOG_WRITER_HPP_ */
