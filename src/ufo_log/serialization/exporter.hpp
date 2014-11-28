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

#ifndef UFO_LOG_EXPORTER_HPP_
#define UFO_LOG_EXPORTER_HPP_

#include <ufo_log/serialization/header.hpp>
#include <ufo_log/serialization/integral.hpp>
#include <ufo_log/serialization/non_integral.hpp>
#include <ufo_log/serialization/non_numeric.hpp>
#include <ufo_log/serialization/importer_exporter.hpp>

namespace ufo { namespace ser {
//------------------------------------------------------------------------------
class exporter : public importer_exporter<u8>
{
public:
    //--------------------------------------------------------------------------
    struct null_type {};
    //--------------------------------------------------------------------------
    static uword bytes_required (null_type) { return 0; }
    //--------------------------------------------------------------------------
    static uword bytes_required (header_data v)
    {
        return header::bytes_required (v);
    }
    //--------------------------------------------------------------------------
    template <class T>
    static typename enable_if_integral<T, uword>::type
    bytes_required (T val)
    {
        return integral::bytes_required (val);
    }
    //--------------------------------------------------------------------------
    template <class T>
    static typename enable_float_double_bool<T, uword>::type
    bytes_required (T val)
    {
        return non_integral::bytes_required (val);
    }
    //--------------------------------------------------------------------------
    static uword bytes_required (const char* str)
    {
        return non_numeric::bytes_required (str);
    }
    //--------------------------------------------------------------------------
    static uword bytes_required (deep_copy_bytes b)
    {
        return non_numeric::bytes_required (b);
    }
    //--------------------------------------------------------------------------
    static uword bytes_required (deep_copy_string s)
    {
        return non_numeric::bytes_required (s);
    }
    //--------------------------------------------------------------------------
    static null_type get_field (null_type, uword)
    {
        return null_type();
    }
    //--------------------------------------------------------------------------
    static header::field get_field (header_data v, uword bytes_required)
    {
        return header::get_field (v, bytes_required);
    }
    //--------------------------------------------------------------------------
    template <class T>
    static typename enable_if_integral<T, integral::field>::type
    get_field (T val, uword bytes_required)
    {
        return integral::get_field (val, bytes_required);
    }
    //--------------------------------------------------------------------------
    template <class T>
    static typename enable_float_double_bool<T, non_integral::field>::type
    get_field (T val, uword bytes_required)
    {
        return non_integral::get_field (val, bytes_required);
    }
    //--------------------------------------------------------------------------
    static non_numeric::field get_field(
            deep_copy_bytes b, uword bytes_required
            )
    {
        return non_numeric::get_field (b, bytes_required);
    }
    //--------------------------------------------------------------------------
    static non_numeric::field get_field(
            deep_copy_string s, uword bytes_required
            )
    {
        return non_numeric::get_field (s, bytes_required);
    }
    //--------------------------------------------------------------------------
    static non_numeric::field get_field (const char*, uword bytes_required)
    {
        return non_numeric::get_field (nullptr, bytes_required);
    }
    //--------------------------------------------------------------------------
    void do_export (null_type, null_type) {}
    //--------------------------------------------------------------------------
    void do_export (header::field f, header_data hd)
    {
        assert (m_pos && m_beg && m_end);
        m_pos = header::encode (m_pos, m_end, f, hd);
    }
    //--------------------------------------------------------------------------
    template <class T>
    typename enable_if_integral<T, void>::type
    do_export (integral::field f, T val)
    {
        m_pos = integral::encode (m_pos, m_end, f, val);
    };
    //--------------------------------------------------------------------------
    template <class T>
    typename enable_float_double_bool<T, void>::type
    do_export (non_integral::field f, T val)
    {
        m_pos = non_integral::encode (m_pos, m_end, f, val);
    };
    //--------------------------------------------------------------------------
    void do_export (non_numeric::field f, const char* str)
    {
        m_pos = non_numeric::encode (m_pos, m_end, f, str);
    }
    //--------------------------------------------------------------------------
    void do_export (non_numeric::field f, deep_copy_bytes b)
    {
        m_pos = non_numeric::encode (m_pos, m_end, f, b);
    }
    //--------------------------------------------------------------------------
    void do_export (non_numeric::field f, deep_copy_string s)
    {
        m_pos = non_numeric::encode (m_pos, m_end, f, s);
    }
    //--------------------------------------------------------------------------
};
//------------------------------------------------------------------------------
}} //namespaces
//------------------------------------------------------------------------------


#endif /* UFO_LOG_EXPORTER_HPP_ */
