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

#ifndef UFO_LOG_IMPORTER_HPP_
#define UFO_LOG_IMPORTER_HPP_


#include <ufo_log/serialization/printf_modifiers.hpp>
#include <ufo_log/serialization/byte_stream_convert.hpp>
#include <ufo_log/serialization/importer_decoder.hpp>
#include <ufo_log/output.hpp>
#include <ufo_log/format_tokens.hpp>

namespace ufo { namespace ser {

//------------------------------------------------------------------------------
class importer : private importer_decoder
{
public:
    //--------------------------------------------------------------------------
    importer()
    {
        m_fmt       = nullptr;
        m_fmt_modif = 0;
        prints_severity = prints_timestamp = true;
    }
    //--------------------------------------------------------------------------
    bool import (output& o, const u8* msg)
    {
        assert (msg);

        init (msg);

        header_data h;
        do_import (h);
        set_next_msg_fmt_string (h.fmt);
        o.entry_begin (h.severity);

        if (h.has_tstamp && prints_timestamp)
        {
            write_timestamp (o, h.tstamp);
        }
        if (prints_severity)
        {
            write_severity (o, h.severity);
        }
        uword params   = h.arity;
        bool fmt_param = find_param_in_fmt_str (o, params > 0);
        while (params && fmt_param)
        {
            consume_next (o, true);
            --params;
            fmt_param = find_param_in_fmt_str (o, params > 0);
        }

        if (params)
        {
            do
            {
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
        if (fmt_param)
        {
            do
            {
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

        o.entry_end();
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
        decoding_field d;
        import_type (d);
        if (d.gen.fclass == ufo_numeric)
        {
            if (d.gen.nclass == ufo_integral)
            {
                output_integral (o, d.num_int, has_placeholder);
            }
            else
            {
                assert (d.gen.nclass == ufo_non_integral);
                output_non_integral (o, d.nom_no_int, has_placeholder);
            }
        }
        else
        {
            assert (d.gen.fclass == ufo_non_numeric);
            output_non_numeric (o, d.no_num, has_placeholder);
        }
    }
    //--------------------------------------------------------------------------
    void output_integral (output& o, integral_field f, bool has_placeholder)
    {
        uword type = f.original_type;
        type      += ((uword) f.is_negative) * 4;

        switch (type)
        {
        case 0: return output_int_type<u8,  u8_modif>  (o, f, has_placeholder);
        case 1: return output_int_type<u16, u16_modif> (o, f, has_placeholder);
        case 2: return output_int_type<u32, u32_modif> (o, f, has_placeholder);
        case 3: return output_int_type<u64, u64_modif> (o, f, has_placeholder);
        case 4: return output_int_type<i8,  i8_modif>  (o, f, has_placeholder);
        case 5: return output_int_type<i16, i16_modif> (o, f, has_placeholder);
        case 6: return output_int_type<i32, i32_modif> (o, f, has_placeholder);
        case 7: return output_int_type<i64, i64_modif> (o, f, has_placeholder);
        default: return;
        }
    }
    //--------------------------------------------------------------------------
    void output_non_integral(
            output& o, non_integral_field f, bool has_placeholder
            )
    {
        switch (f.niclass)
        {
        case ufo_double:
            return output_floating_type<double, u64, double_modif>(
                    o, f, has_placeholder
                    );
        case ufo_float :
            return output_floating_type<float, u32, float_modif>(
                                            o, f, has_placeholder
                                            );
        case ufo_bool  :
        {
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
            output& o, non_numeric_field f, bool has_placeholder
            )
    {
        switch (f.nnclass)
        {
        case ufo_c_str:
        {
            literal_wrapper l;
            do_import (l, f);
            if (has_placeholder && l.lit)
            {
                o.write (l.lit);
            }
            else if (!l.lit)
            {
                static const char lit_null_err[] =
                    "([logger err]->nullptr literal, ignored)";
                o.write (lit_null_err, sizeof lit_null_err - 1);
                assert (false && "null literal");
            }
            break;
        }
        case ufo_ptr:
        {
            static_assert (sizeof (uword) == 4 || sizeof (uword) == 8, "");
            const char* fmt = (sizeof (uword) == 4) ?
                                    u32_modif::hex : u64_modif::hex;
            ptr_wrapper p;
            do_import (p, f);
            uword v = (uword) p.ptr;

            if (has_placeholder)
            {
                output_num (o, v, fmt);
            }
            break;
        }
        case ufo_deep_copied_str :
        {
            deep_copy_string str;
            do_import (str, f);
            if (has_placeholder)
            {
                o.write (str.mem, str.size);
            }
            break;
        }
        case ufo_deep_copied_mem  :
        {
            deep_copy_bytes bytes;
            do_import (bytes, f);
            if (has_placeholder)
            {
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
    template <class T, class fmt_struct>
    void output_int_type (output& o, integral_field f, bool has_placeholder)
    {
        T v;
        do_import (v, f);
        const char* fmt;
        switch (m_fmt_modif)
        {
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
            if (has_placeholder)
            {
                write_invalid_modifier (o);
            }
            fmt = fmt_struct::norm;
            break;
        }
        if (has_placeholder)
        {
            output_num (o, v, fmt);
        }
    }
    //--------------------------------------------------------------------------
    template <class T, class H, class fmt_struct>
    void output_floating_type(
            output& o, non_integral_field f, bool has_placeholder)
    {
        union hex_hack
        {
            T floating;
            H hex;
        };
        hex_hack v;
        do_import (v.floating, f);
        const char* fmt;
        switch (m_fmt_modif)
        {
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
            if (has_placeholder)
            {
                write_invalid_modifier (o);
            }
            fmt = fmt_struct::norm;
            break;
        }
        if (has_placeholder)
        {
            if (fmt != fmt_struct::hex)
            {
                output_num (o, v.floating, fmt);
            }
            else
            {
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
        while (m_fmt != nullptr)
        {
            auto found = std::strchr (m_fmt, fmt::placeholder_open);
            if (found && (found + 1) != 0)
            {
                m_fmt       = found + 1;
                m_fmt_modif = *m_fmt;
            }
            else
            {
                o.write (fmt_prev);
                m_fmt_modif = 0;
                return false;
            }

            if (*m_fmt == fmt::placeholder_close)
            {
                m_fmt_modif = 0;
                ++m_fmt;
                o.write (fmt_prev, found - fmt_prev);
            }
            else if (*(m_fmt + 1) == fmt::placeholder_close)
            {
                m_fmt += 2;
                o.write (fmt_prev, found - fmt_prev);
            }
            else { continue; }

            if (!remaining_parameters)
            {
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
        uword adv             = std::snprintf (buff, sizeof buff, fmt, val);
        buff[sizeof buff - 1] = 0;
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
        output_num (o, t, "[%020llu] ");
    }
    //--------------------------------------------------------------------------

    const char* m_fmt;
    char        m_fmt_modif;
};
//------------------------------------------------------------------------------
}} //namespaces
//------------------------------------------------------------------------------


#endif /* UFO_LOG_IMPORTER_HPP_ */
