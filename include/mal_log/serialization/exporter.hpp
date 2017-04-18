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

#ifndef MAL_LOG_EXPORTER_HPP_
#define MAL_LOG_EXPORTER_HPP_

#include <mal_log/util/system.hpp>
#include <mal_log/util/integer.hpp>
#include <mal_log/frontend_types.hpp>
#include <mal_log/serialization/fields.hpp>
#include <mal_log/serialization/header_data.hpp>
#include <mal_log/serialization/basic_encoder_decoder.hpp>
#include <mal_log/serialization/importer_exporter.hpp>
#include <mal_log/util/integral_enable_if.hpp>
#include <mal_log/util/opaque_pod.hpp>

#ifndef MAL_NO_VARIABLE_INTEGER_WIDTH
    #include <mal_log/util/integer_bits.hpp>
#endif

namespace mal { namespace ser {
//------------------------------------------------------------------------------
class exporter : public importer_exporter<u8>, public basic_encoder_decoder
{
public:
    //--------------------------------------------------------------------------
    struct null_type {};
    //--------------------------------------------------------------------------
    static uword bytes_required (null_type) { return 0; }
    //--------------------------------------------------------------------------
    static uword bytes_required (header_data h)
    {
        return sizeof (header_field) +
               sizeof (const char*) +
               (h.has_tstamp      ? unsigned_bytes_required (h.tstamp) : 0) +
               (h.sync == nullptr ? 0 : sizeof h.sync );
    }
    //--------------------------------------------------------------------------
    template <class T>
    static typename enable_if_unsigned<T, uword>::type
    bytes_required (T val)                                                      //todo: make constextr...
    {
        return unsigned_bytes_required (val) + sizeof (integral_field);
    }
    //--------------------------------------------------------------------------
    template <class T>
    static typename enable_if_signed<T, uword>::type
    bytes_required (T val)                                                      //todo make constexpr
    {
        typedef typename std::make_unsigned<T>::type U;
        val = (val >= 0) ? val : prepare_negative (val);
        return unsigned_bytes_required ((U) val) + sizeof (integral_field);
    }
    //--------------------------------------------------------------------------
    static uword bytes_required (bool)
    {
        return 0 + sizeof (non_integral_field);
    }
    //--------------------------------------------------------------------------
    static uword bytes_required (float)
    {
        return sizeof (float) + sizeof (non_integral_field);
    }
    //--------------------------------------------------------------------------
    static uword bytes_required (double)
    {
        return sizeof (double) + sizeof (non_integral_field);
    }
    //--------------------------------------------------------------------------
    static uword bytes_required (literal_wrapper str)
    {
        return sizeof (non_integral_field) + sizeof literal_wrapper().lit;
    }
    //--------------------------------------------------------------------------
    static uword bytes_required (ptr_wrapper p)
    {
        return sizeof (non_integral_field) + sizeof ptr_wrapper().ptr;
    }
    //--------------------------------------------------------------------------
    static uword bytes_required (deep_copy_bytes b)
    {
        return delimited_bytes_required ((delimited_mem) b);
    }
    //--------------------------------------------------------------------------
    static uword bytes_required (deep_copy_string s)
    {
        return delimited_bytes_required ((delimited_mem) s);
    }
    //--------------------------------------------------------------------------
    static null_type get_field (null_type, uword)
    {
        return null_type();
    }
    //--------------------------------------------------------------------------
    static header_field get_field (header_data v, uword bytes_required)
    {
        uword bytes = bytes_required -
                      sizeof (const char*) -
                      sizeof (header_field) -
                      ((v.sync != nullptr ? 1 : 0) * sizeof v.sync)
                      ;
        assert ((v.has_tstamp && bytes) || !v.has_tstamp);
        header_field f;
        f.arity           = v.arity;
        f.severity        = v.severity;
        f.no_timestamp    = v.has_tstamp      ? 0 : 1;
        f.timestamp_bytes = v.has_tstamp      ? bytes - 1 : 0;
        f.is_sync         = v.sync != nullptr ? 1 : 0;
        return f;
    }
    //--------------------------------------------------------------------------
    template <class T>
    static typename enable_if_unsigned<T, integral_field>::type
    get_field (T val, uword bytes_required)
    {
        return get_integral_field (val, bytes_required, false);
    }
    //--------------------------------------------------------------------------
    template <class T>
    static typename enable_if_signed<T, integral_field>::type
    get_field (T val, uword bytes_required)
    {
        return get_integral_field (val, bytes_required, val < 0);
    }
    //--------------------------------------------------------------------------
    static non_integral_field get_field (bool val, uword bytes_required)
    {
        non_integral_field f;
        f.fclass   = mal_numeric;
        f.nclass   = mal_non_integral;
        f.niclass  = mal_bool;
        f.bool_val = val ? 1 : 0;
        return f;
    }
    //--------------------------------------------------------------------------
    static non_integral_field get_field (float, uword bytes_required)
    {
        non_integral_field f;
        f.fclass   = mal_numeric;
        f.nclass   = mal_non_integral;
        f.niclass  = mal_float;
        f.bool_val = 0;
        return f;
    }
    //--------------------------------------------------------------------------
    static non_integral_field get_field (double, uword bytes_required)
    {
        non_integral_field f;
        f.fclass   = mal_numeric;
        f.nclass   = mal_non_integral;
        f.niclass  = mal_double;
        f.bool_val = 0;
        return f;
    }
    //--------------------------------------------------------------------------
    static non_numeric_field get_field (deep_copy_bytes b, uword bytes_required)
    {
        assert (bytes_required > sizeof (non_numeric_field) + b.size);
        non_numeric_field f;
        f.fclass                   = mal_non_numeric;
        f.nnclass                  = mal_deep_copied_mem;
        f.deep_copied_length_bytes = bytes_required - b.size - sizeof (f) - 1;
        return f;
    }
    //--------------------------------------------------------------------------
    static non_numeric_field get_field(
                                deep_copy_string s, uword bytes_required
                                )
    {
        assert (bytes_required > sizeof (non_numeric_field) + s.size);
        non_numeric_field f;
        f.fclass                   = mal_non_numeric;
        f.nnclass                  = mal_deep_copied_str;
        f.deep_copied_length_bytes = bytes_required - s.size - sizeof (f) - 1;
        return f;
    }
    //--------------------------------------------------------------------------
    static non_numeric_field get_field (literal_wrapper, uword bytes_required)
    {
        non_numeric_field f;
        f.fclass                    = mal_non_numeric;
        f.nnclass                   = mal_c_str;
        f.deep_copied_length_bytes  = 0;
        return f;
    }
    //--------------------------------------------------------------------------
    static non_numeric_field get_field (ptr_wrapper, uword bytes_required)
    {
        non_numeric_field f;
        f.fclass                    = mal_non_numeric;
        f.nnclass                   = mal_ptr;
        f.deep_copied_length_bytes  = 0;
        return f;
    }
    //--------------------------------------------------------------------------
    void do_export (null_type, null_type) {}
    //--------------------------------------------------------------------------
    void do_export (header_data hd, header_field f)
    {
        export_type (f);
        export_type (hd.fmt);
        if (hd.has_tstamp)
        {
            encode_unsigned (hd.tstamp, ((uword) f.timestamp_bytes) + 1);
        }
        if (hd.sync)
        {
            export_type (hd.sync);
        }
    }
    //--------------------------------------------------------------------------
    template <class T>
    typename enable_if_unsigned<T, void>::type
    do_export (T val, integral_field f)
    {
        encode_prepared_integral (val, f);
    };
    //--------------------------------------------------------------------------
    template <class T>
    typename enable_if_signed<T, void>::type
    do_export (T val, integral_field f)
    {
        encode_prepared_integral(
                (f.is_negative == 0) ? val : prepare_negative (val), f
                );
    };
    //--------------------------------------------------------------------------
    void do_export (bool, non_integral_field f)
    {
        export_type (f);
    }
    //--------------------------------------------------------------------------
    void do_export (float v, non_integral_field f)
    {
        export_type (f);
        export_type (v);
    }
    //--------------------------------------------------------------------------
    void do_export (double v, non_integral_field f)
    {
        export_type (f);
        export_type (v);
    }
    //--------------------------------------------------------------------------
    void do_export (literal_wrapper l, non_numeric_field f)
    {
        export_type (f);
        export_type (l.lit);
    }
    //--------------------------------------------------------------------------
    void do_export (ptr_wrapper p, non_numeric_field f)
    {
        export_type (f);
        export_type (p.ptr);
    }
    //--------------------------------------------------------------------------
    void do_export (deep_copy_bytes b, non_numeric_field f)
    {
        encode_delimited ((delimited_mem) b, f);
    }
    //--------------------------------------------------------------------------
    void do_export (deep_copy_string s, non_numeric_field f)
    {
        encode_delimited ((delimited_mem) s, f);
    }
    //--------------------------------------------------------------------------
    opaque_pod<2 * sizeof (uword)> opaque_data;
    //--------------------------------------------------------------------------
private:
    //--------------------------------------------------------------------------
    template <class T>
    void export_type (T val)
    {
        m_pos = encode_type (m_pos, m_end, val);
    }
    //--------------------------------------------------------------------------
    template <class T>
    static typename enable_if_unsigned<T, uword>::type
    unsigned_bytes_required (T val)
    {
#ifndef MAL_NO_VARIABLE_INTEGER_WIDTH
        return highest_used_byte (val) + 1;
#else
        return sizeof (T);
#endif
    }
    //--------------------------------------------------------------------------
    template <class T>
    typename enable_if_unsigned<T, void>::type
    encode_unsigned (T val, uword size)
    {
        assert (m_pos && m_end);
        assert (m_pos + size <= m_end);
        assert (size <= sizeof (T));

        for (uword i = 0; i < size; ++i, ++m_pos) {
            *m_pos = (u8) (val >> (i * 8));
        }
    }
    //--------------------------------------------------------------------------
    template <class T>
    static typename enable_if_signed<T, T>::type
    prepare_negative (T val)
    {
        typedef typename std::make_unsigned<T>::type U;
        return (T) (~((U) val));
    }
    //--------------------------------------------------------------------------
    template <class T>
    static integral_field get_integral_field(
                            T val, uword bytes_required, bool negative
                            )
    {
        static_assert(
            sizeof (T) <= 8,
            "f.original type needs a proper compile time log2"
            );
        assert (bytes_required <= (sizeof (T) + sizeof (integral_field)));
        integral_field f;
        f.fclass        = mal_numeric;
        f.nclass        = mal_integral;
        f.bytes         = (bytes_required - 1 - sizeof f);
        assert (f.bytes == (bytes_required - 1 - sizeof f));
        f.original_type = ((sizeof val / 2) == 4) ? 3 : (sizeof val / 2);
        f.is_negative   = negative ? 1 : 0;
        return f;
    }
    //--------------------------------------------------------------------------
    template <class T>
    void encode_prepared_integral (T val, integral_field f)
    {
        typedef typename std::make_unsigned<T>::type U;
        export_type (f);
        encode_unsigned ((U) val, ((uword) f.bytes) + 1);
    }
    //--------------------------------------------------------------------------
    static uword delimited_bytes_required (delimited_mem m)
    {
        return sizeof (non_numeric_field) +
                unsigned_bytes_required (m.size) +
                m.size
                ;
    }
    //--------------------------------------------------------------------------
    void encode_delimited (delimited_mem m, non_numeric_field f)
    {
        export_type (f);
        encode_unsigned(
                m.size, ((uword) f.deep_copied_length_bytes) + 1
                );
        assert (m_pos + m.size <= m_end);
        std::memcpy (m_pos, m.mem, m.size);
        m_pos += m.size;
    }
    //--------------------------------------------------------------------------
};
//------------------------------------------------------------------------------
}} //namespaces
//------------------------------------------------------------------------------

#endif /* MAL_LOG_EXPORTER_HPP_ */
