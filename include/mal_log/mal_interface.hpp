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

#ifndef MAL_LOG_INTERFACE_HPP_
#define MAL_LOG_INTERFACE_HPP_

#include <memory>
#include <cassert>
#include <string>
#include <type_traits>
#include <mal_log/frontend.hpp>
#include <mal_log/timestamp.hpp>
#include <mal_log/serialization/exporter.hpp>
#include <mal_log/sync_point.hpp>
#include <mal_log/util/side_effect_assert.hpp>

namespace mal {

//------------------------------------------------------------------------------
template <class T, class field>
inline bool prebuild_data (T& v, field& f, uword& total_length)
{
    typedef ser::exporter exp;
    auto clength   = exp::bytes_required (v);
    f              = exp::get_field (v, clength);
    total_length  += clength;
    return (total_length >= (total_length - clength));
}
//------------------------------------------------------------------------------
// todo: this function has to be beautified, but I don't want to add compile
// time vectors or more complexity right now. This is written for compilers
// without variadic template args.
//------------------------------------------------------------------------------
template<
    bool is_async,
    class A,
    class B,
    class C,
    class D,
    class E,
    class F,
    class G,
    class H,
    class I,
    class J,
    class K,
    class L,
    class M,
    class N
    >
bool new_entry(                                                                 //don't use this directly, use the macros!
    frontend&     fe,
    sev::severity sv,
    const char*   fmt,
    A             a,
    B             b,
    C             c,
    D             d,
    E             e,
    F             f,
    G             g,
    H             h,
    I             i,
    J             j,
    K             k,
    L             l,
    M             m,
    N             n
    )
{
    typedef ser::exporter exp;
    typedef exp::null_type null_type;

    const uword arity =
            (std::is_same<A, null_type>::value ? 0 : 1) +
            (std::is_same<B, null_type>::value ? 0 : 1) +
            (std::is_same<C, null_type>::value ? 0 : 1) +
            (std::is_same<D, null_type>::value ? 0 : 1) +
            (std::is_same<E, null_type>::value ? 0 : 1) +
            (std::is_same<F, null_type>::value ? 0 : 1) +
            (std::is_same<G, null_type>::value ? 0 : 1) +
            (std::is_same<H, null_type>::value ? 0 : 1) +
            (std::is_same<I, null_type>::value ? 0 : 1) +
            (std::is_same<J, null_type>::value ? 0 : 1) +
            (std::is_same<K, null_type>::value ? 0 : 1) +
            (std::is_same<L, null_type>::value ? 0 : 1) +
            (std::is_same<M, null_type>::value ? 0 : 1) +
            (std::is_same<N, null_type>::value ? 0 : 1)
            ;

    ser::header_data hdr;

    decltype (exp::get_field (a, 0))   a_field;                                 //just 16 bytes (if all parameters are used)
    decltype (exp::get_field (b, 0))   b_field;
    decltype (exp::get_field (c, 0))   c_field;
    decltype (exp::get_field (d, 0))   d_field;
    decltype (exp::get_field (e, 0))   e_field;
    decltype (exp::get_field (f, 0))   f_field;
    decltype (exp::get_field (g, 0))   g_field;
    decltype (exp::get_field (h, 0))   h_field;
    decltype (exp::get_field (i, 0))   i_field;
    decltype (exp::get_field (j, 0))   j_field;
    decltype (exp::get_field (k, 0))   k_field;
    decltype (exp::get_field (l, 0))   l_field;
    decltype (exp::get_field (m, 0))   m_field;
    decltype (exp::get_field (n, 0))   n_field;
    decltype (exp::get_field (hdr, 0)) hdr_field;

    uword length = 0;

    mal_side_effect_assert (prebuild_data (a, a_field, length));
    mal_side_effect_assert (prebuild_data (b, b_field, length));
    mal_side_effect_assert (prebuild_data (c, c_field, length));
    mal_side_effect_assert (prebuild_data (d, d_field, length));
    mal_side_effect_assert (prebuild_data (e, e_field, length));
    mal_side_effect_assert (prebuild_data (f, f_field, length));
    mal_side_effect_assert (prebuild_data (g, g_field, length));
    mal_side_effect_assert (prebuild_data (h, h_field, length));
    mal_side_effect_assert (prebuild_data (i, i_field, length));
    mal_side_effect_assert (prebuild_data (j, j_field, length));
    mal_side_effect_assert (prebuild_data (k, k_field, length));
    mal_side_effect_assert (prebuild_data (l, l_field, length));
    mal_side_effect_assert (prebuild_data (m, m_field, length));
    mal_side_effect_assert (prebuild_data (n, n_field, length));

    auto td = fe.get_timestamp_data();
    hdr     = ser::make_header_data (sv, fmt, arity, td.producer_timestamps);
    if (hdr.has_tstamp) {                                                        //timestamping is actually slow! in my machine slows down the producers by a factor of 2
        hdr.tstamp = get_ns_timestamp() - td.base;
    }
    sync_point sync;
    if (!is_async) {
        hdr.sync = &sync;
    }
    mal_side_effect_assert (prebuild_data (hdr, hdr_field, length));

    auto enc = fe.get_encoder (length, sv);
    if (enc.has_memory()) {
        enc.do_export (hdr, hdr_field);
        enc.do_export (a, a_field);
        enc.do_export (b, b_field);
        enc.do_export (c, c_field);
        enc.do_export (d, d_field);
        enc.do_export (e, e_field);
        enc.do_export (f, f_field);
        enc.do_export (g, g_field);
        enc.do_export (h, h_field);
        enc.do_export (i, i_field);
        enc.do_export (j, j_field);
        enc.do_export (k, k_field);
        enc.do_export (l, l_field);
        enc.do_export (m, m_field);
        enc.do_export (n, n_field);
        if (is_async) {
            fe.async_push_encoded (enc);
            return true;
        }
        else {
            return fe.sync_push_encoded (enc, sync);
        }
    }
    return false;
}
//------------------------------------------------------------------------------
template<
    bool is_async,
    class A,
    class B,
    class C,
    class D,
    class E,
    class F,
    class G,
    class H,
    class I,
    class J,
    class K,
    class L,
    class M
    >
bool new_entry(                                                                 //don't use this directly, use the macros!
    frontend&     fe,
    sev::severity sv,
    const char*   fmt,
    A             a,
    B             b,
    C             c,
    D             d,
    E             e,
    F             f,
    G             g,
    H             h,
    I             i,
    J             j,
    K             k,
    L             l,
    M             m
    )
{
    ser::exporter::null_type no;
    return new_entry<is_async>(
            fe, sv, fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, no
            );
}
//------------------------------------------------------------------------------
template<
    bool is_async,
    class A,
    class B,
    class C,
    class D,
    class E,
    class F,
    class G,
    class H,
    class I,
    class J,
    class K,
    class L
    >
bool new_entry(                                                                 //don't use this directly, use the macros!
    frontend&     fe,
    sev::severity sv,
    const char*   fmt,
    A             a,
    B             b,
    C             c,
    D             d,
    E             e,
    F             f,
    G             g,
    H             h,
    I             i,
    J             j,
    K             k,
    L             l
    )
{
    ser::exporter::null_type no;
    return new_entry<is_async>(
            fe, sv, fmt, a, b, c, d, e, f, g, h, i, j, k, l, no, no
            );
}
//------------------------------------------------------------------------------
template<
    bool is_async,
    class A,
    class B,
    class C,
    class D,
    class E,
    class F,
    class G,
    class H,
    class I,
    class J,
    class K
    >
bool new_entry(                                                                 //don't use this directly, use the macros!
    frontend&     fe,
    sev::severity sv,
    const char*   fmt,
    A             a,
    B             b,
    C             c,
    D             d,
    E             e,
    F             f,
    G             g,
    H             h,
    I             i,
    J             j,
    K             k
    )
{
    ser::exporter::null_type no;
    return new_entry<is_async>(
            fe, sv, fmt, a, b, c, d, e, f, g, h, i, j, k, no, no, no
            );
}
//------------------------------------------------------------------------------
template<
    bool is_async,
    class A,
    class B,
    class C,
    class D,
    class E,
    class F,
    class G,
    class H,
    class I,
    class J
    >
bool new_entry(                                                                 //don't use this directly, use the macros!
    frontend&     fe,
    sev::severity sv,
    const char*   fmt,
    A             a,
    B             b,
    C             c,
    D             d,
    E             e,
    F             f,
    G             g,
    H             h,
    I             i,
    J             j
    )
{
    ser::exporter::null_type no;
    return new_entry<is_async>(
            fe, sv, fmt, a, b, c, d, e, f, g, h, i, j, no, no, no, no
            );
}
//------------------------------------------------------------------------------
template<
    bool is_async,
    class A,
    class B,
    class C,
    class D,
    class E,
    class F,
    class G,
    class H,
    class I
    >
bool new_entry(                                                                 //don't use this directly, use the macros!
    frontend&     fe,
    sev::severity sv,
    const char*   fmt,
    A             a,
    B             b,
    C             c,
    D             d,
    E             e,
    F             f,
    G             g,
    H             h,
    I             i
    )
{
    ser::exporter::null_type no;
    return new_entry<is_async>(
            fe, sv, fmt, a, b, c, d, e, f, g, h, i, no, no, no, no, no
            );
}
//------------------------------------------------------------------------------
template<
    bool is_async,
    class A,
    class B,
    class C,
    class D,
    class E,
    class F,
    class G,
    class H
    >
bool new_entry(                                                                 //don't use this directly, use the macros!
    frontend&     fe,
    sev::severity sv,
    const char*   fmt,
    A             a,
    B             b,
    C             c,
    D             d,
    E             e,
    F             f,
    G             g,
    H             h
    )
{
    ser::exporter::null_type no;
    return new_entry<is_async>(
            fe, sv, fmt, a, b, c, d, e, f, g, h, no, no, no, no, no, no
            );
}
//------------------------------------------------------------------------------
template<
    bool is_async,
    class A,
    class B,
    class C,
    class D,
    class E,
    class F,
    class G
    >
bool new_entry(                                                                 //don't use this directly, use the macros!
    frontend&     fe,
    sev::severity sv,
    const char*   fmt,
    A             a,
    B             b,
    C             c,
    D             d,
    E             e,
    F             f,
    G             g
    )
{
    ser::exporter::null_type no;
    return new_entry<is_async>(
            fe, sv, fmt, a, b, c, d, e, f, g, no, no, no, no, no, no, no
            );
}
//------------------------------------------------------------------------------
template<bool is_async, class A, class B, class C, class D, class E, class F>
bool new_entry(                                                                 //don't use this directly, use the macros!
    frontend&     fe,
    sev::severity sv,
    const char*   fmt,
    A             a,
    B             b,
    C             c,
    D             d,
    E             e,
    F             f
    )
{
    ser::exporter::null_type no;
    return new_entry<is_async>(
            fe, sv, fmt, a, b, c, d, e, f, no, no, no, no, no, no, no, no
            );
}
//------------------------------------------------------------------------------
template<bool is_async, class A, class B, class C, class D, class E>
bool new_entry(                                                                 //don't use this directly, use the macros!
    frontend& fe, sev::severity sv, const char* fmt, A a, B b, C c, D d, E e
    )
{
    ser::exporter::null_type no;
    return new_entry<is_async>(
            fe, sv, fmt, a, b, c, d, e, no, no, no, no, no, no, no, no, no
            );
}
//------------------------------------------------------------------------------
template<bool is_async, class A, class B, class C, class D>
bool new_entry(                                                                 //don't use this directly, use the macros!
    frontend& fe, sev::severity sv, const char* fmt, A a, B b, C c, D d
    )
{
    ser::exporter::null_type no;
    return new_entry<is_async>(
            fe, sv, fmt, a, b, c, d, no, no, no, no, no, no, no, no, no, no
            );
}
//------------------------------------------------------------------------------
template<bool is_async, class A, class B, class C>
bool new_entry (frontend& fe, sev::severity sv, const char* fmt, A a, B b, C c)
{
    ser::exporter::null_type no;
    return new_entry<is_async>(
        fe, sv, fmt, a, b, c, no, no, no, no, no, no, no, no, no, no, no
        );
}
//------------------------------------------------------------------------------
template<bool is_async, class A, class B>
bool new_entry (frontend& fe, sev::severity sv, const char* fmt, A a, B b)
{
    ser::exporter::null_type no;
    return new_entry<is_async>(
        fe, sv, fmt, a, b, no, no, no, no, no, no, no, no, no, no, no, no
        );
}
//------------------------------------------------------------------------------
template<bool is_async, class A>
bool new_entry (frontend& fe, sev::severity sv, const char* fmt, A a)
{
    ser::exporter::null_type no;
    return new_entry<is_async>(
        fe, sv, fmt, a, no, no, no, no, no, no, no, no, no, no, no, no, no
        );
}
//------------------------------------------------------------------------------
template<bool is_async>
bool new_entry (frontend& fe, sev::severity sv, const char* fmt)
{
    ser::exporter::null_type no;
    return new_entry<is_async>(
        fe, sv, fmt, no, no, no, no, no, no, no, no, no, no, no, no, no, no
        );
}
//------------------------------------------------------------------------------
} //namespace

#endif /* MAL_LOG_INTERFACE_HPP_ */
