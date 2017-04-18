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
#ifndef MAL_LOG_IMPORTER_HPP_
#define MAL_LOG_IMPORTER_HPP_

#include <mal_log/serialization/basic_encoder_decoder.hpp>
#include <mal_log/frontend_types.hpp>
#include <mal_log/serialization/importer_exporter.hpp>
#include <mal_log/util/placement_new.hpp>

namespace mal { namespace ser {

//------------------------------------------------------------------------------
class importer : protected importer_exporter<const u8>,
                 private basic_encoder_decoder
{
public:
    //--------------------------------------------------------------------------
    template <class T>
    void import_type (T& t)
    {
        m_pos = decode_type (t, m_pos, m_end);
    }
    //--------------------------------------------------------------------------
    void do_import (header_data& hd)
    {
        header_field h;
        import_type (h);
        import_type (hd.fmt);

        hd.arity      = h.arity;
        hd.has_tstamp = h.no_timestamp ? false : true;
        hd.tstamp     = 0;
        hd.severity   = (sev::severity) h.severity;
        hd.sync       = nullptr;

        if (hd.has_tstamp) {
            decode_unsigned (hd.tstamp, ((uword) h.timestamp_bytes) + 1);
        }
        if (h.is_sync) {
            import_type (hd.sync);
        }
    }
    //--------------------------------------------------------------------------
    template <class T>
    typename enable_if_unsigned<T, void>::type
    do_import (T& val, integral_field f)
    {
        decode_unsigned (val, ((uword) f.bytes) + 1);
    }
    //--------------------------------------------------------------------------
    template <class T>
    typename enable_if_signed<T, void>::type
    do_import (T& val, integral_field f)
    {
        decode_unsigned (val, ((uword) f.bytes) + 1);
        val = (f.is_negative == 0) ? val : reconstruct_negative (val);
    }
    //--------------------------------------------------------------------------
    void do_import (double& val, non_integral_field f)
    {
        import_type (val);
    }
    //--------------------------------------------------------------------------
    void do_import (float& val, non_integral_field f)
    {
        import_type (val);
    }
    //--------------------------------------------------------------------------
    void do_import (bool& val, non_integral_field f)
    {
        val = f.bool_val ? true : false;
    }
    //--------------------------------------------------------------------------
    void do_import (literal_wrapper& l, non_numeric_field f)
    {
        import_type (l.lit);
    }
    //--------------------------------------------------------------------------
    void do_import (ptr_wrapper& p, non_numeric_field f)
    {
        import_type (p.ptr);
    }
    //--------------------------------------------------------------------------
    void do_import (deep_copy_bytes& b, non_numeric_field f)
    {
        decode_delimited ((delimited_mem&) b, f);
    }
    //--------------------------------------------------------------------------
    void do_import (deep_copy_string& s, non_numeric_field f)
    {
        decode_delimited ((delimited_mem&) s, f);
    }
    //--------------------------------------------------------------------------
    template <class T>
    static typename enable_if_signed<T, T>::type
    reconstruct_negative (T val)
    {
        typedef typename std::make_unsigned<T>::type U;
        return (T) (~((U) val));
    }
    //--------------------------------------------------------------------------
    template <class T>
    void decode_unsigned (T& val, uword size)
    {
        assert (m_pos + size <= m_end);
        val = 0;
        for (uword i = 0; i < size; ++i, ++m_pos) {
            val |= ((T) *m_pos) << (i * 8);
        }
    }
    //--------------------------------------------------------------------------
    void decode_delimited (delimited_mem& m, non_numeric_field f)
    {
        decode_unsigned (m.size, ((uword) f.deep_copied_length_bytes) + 1);
        m.mem = m_pos;
        assert (m_pos + m.size <= m_end);
        m_pos += m.size;
    }
    //--------------------------------------------------------------------------
};

}} //namespaces

#endif /* MAL_LOG_IMPORTER_HPP_ */
