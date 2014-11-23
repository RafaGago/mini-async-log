/*
 * log_interface.hpp
 *
 *  Created on: Nov 11, 2014
 *      Author: rafgag
 */


#ifndef UFO_LOG_INTERFACE_HPP_
#define UFO_LOG_INTERFACE_HPP_

#include <cassert>
#include <string>
#include <type_traits>
#include <ufo_log/protocol.hpp>
#include <ufo_log/frontend.hpp>
#include <ufo_log/message_encoder.hpp>

namespace ufo {

//------------------------------------------------------------------------------
proto::raw_data deep_copy (const std::string& str)
{
    proto::raw_data r;
    r.mem  = &str[0];
    r.size = str.size();
    return r;
}
//------------------------------------------------------------------------------
proto::raw_data deep_copy (const void* mem, uword size)
{
    proto::raw_data r;
    r.mem  = mem;
    r.size = size;
    return r;
}
//------------------------------------------------------------------------------
template <uword N>
proto::str_literal lit (const char (&arr)[N])
{
    return proto::str_literal (arr);
}
//------------------------------------------------------------------------------
template <class T>
proto::integer<T> hex (T v)
{
    return proto::integer<T> (v, true);
}
//------------------------------------------------------------------------------
template <class T>
proto::integer<uword> ptr (T* v)
{
    return proto::integer<uword> ((uword) v, true);
}
//------------------------------------------------------------------------------
proto::byte_stream bytes (const void* mem, uword size)
{
    proto::byte_stream b;
    b.mem  = mem;
    b.size = size;
    return b;
}
//------------------------------------------------------------------------------
void log_error_call (const char* str)
{
    //todo
}
//------------------------------------------------------------------------------
template<
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
    frontend&          fe,
    sev::severity      sv,
    proto::str_literal fmt,
    A                  a,
    B                  b,
    C                  c,
    D                  d,
    E                  e,
    F                  f,
    G                  g,
    H                  h,
    I                  i,
    J                  j,
    K                  k,
    L                  l,
    M                  m,
    N                  n
    )
{
    typedef proto::encoder::null_type null_type;
    const uword arity = 1 +
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

    uword length = proto::encoder::required_bytes_arity1(); //1
    length      += proto::encoder::required_bytes (a);      //2
    length      += proto::encoder::required_bytes (b);      //3
    length      += proto::encoder::required_bytes (c);      //4
    length      += proto::encoder::required_bytes (d);      //5
    length      += proto::encoder::required_bytes (e);      //6
    length      += proto::encoder::required_bytes (f);      //7
    length      += proto::encoder::required_bytes (g);      //8
    length      += proto::encoder::required_bytes (h);      //9
    length      += proto::encoder::required_bytes (i);      //10
    length      += proto::encoder::required_bytes (j);      //11
    length      += proto::encoder::required_bytes (k);      //12
    length      += proto::encoder::required_bytes (l);      //13
    length      += proto::encoder::required_bytes (m);      //14
    length      += proto::encoder::required_bytes (n);      //15
    auto enc = fe.get_encoder (length);
    if (enc.can_encode())
    {
        enc.encode_basic (sv, arity, fmt);
        if (!enc.encode (a)) { goto overflow; }
        if (!enc.encode (b)) { goto overflow; }
        if (!enc.encode (c)) { goto overflow; }
        if (!enc.encode (d)) { goto overflow; }
        if (!enc.encode (e)) { goto overflow; }
        if (!enc.encode (f)) { goto overflow; }
        if (!enc.encode (g)) { goto overflow; }
        if (!enc.encode (h)) { goto overflow; }
        if (!enc.encode (i)) { goto overflow; }
        if (!enc.encode (j)) { goto overflow; }
        if (!enc.encode (k)) { goto overflow; }
        if (!enc.encode (l)) { goto overflow; }
        if (!enc.encode (m)) { goto overflow; }
        if (!enc.encode (n)) { goto overflow; }
        fe.push_encoded (enc);
        return true;
    }
    log_error_call ("couldn't allocate encoder\n");
    return false;
overflow:
    fe.push_encoded (enc);
    log_error_call ("overflow when encoding\n");
    assert (false && "bug!");                                                   //The size is precomputed, so this should be unreachable.
    return false;
}
//------------------------------------------------------------------------------
template<
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
    frontend&          fe,
    sev::severity      sv,
    proto::str_literal fmt,
    A                  a,
    B                  b,
    C                  c,
    D                  d,
    E                  e,
    F                  f,
    G                  g,
    H                  h,
    I                  i,
    J                  j,
    K                  k,
    L                  l,
    M                  m
    )
{
    proto::encoder::null_type no;
    return new_entry (fe, sv, fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, no);
}
//------------------------------------------------------------------------------
template<
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
    frontend&          fe,
    sev::severity      sv,
    proto::str_literal fmt,
    A                  a,
    B                  b,
    C                  c,
    D                  d,
    E                  e,
    F                  f,
    G                  g,
    H                  h,
    I                  i,
    J                  j,
    K                  k,
    L                  l
    )
{
    proto::encoder::null_type no;
    return new_entry (fe, sv, fmt, a, b, c, d, e, f, g, h, i, j, k, l, no, no);
}
//------------------------------------------------------------------------------
template<
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
    frontend&          fe,
    sev::severity      sv,
    proto::str_literal fmt,
    A                  a,
    B                  b,
    C                  c,
    D                  d,
    E                  e,
    F                  f,
    G                  g,
    H                  h,
    I                  i,
    J                  j,
    K                  k
    )
{
    proto::encoder::null_type no;
    return new_entry (fe, sv, fmt, a, b, c, d, e, f, g, h, i, j, k, no, no, no);
}
//------------------------------------------------------------------------------
template<
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
    frontend&          fe,
    sev::severity      sv,
    proto::str_literal fmt,
    A                  a,
    B                  b,
    C                  c,
    D                  d,
    E                  e,
    F                  f,
    G                  g,
    H                  h,
    I                  i,
    J                  j
    )
{
    proto::encoder::null_type no;
    return new_entry(
        fe, sv, fmt, a, b, c, d, e, f, g, h, i, j, no, no, no, no
        );
}
//------------------------------------------------------------------------------
template<
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
    frontend&          fe,
    sev::severity      sv,
    proto::str_literal fmt,
    A                  a,
    B                  b,
    C                  c,
    D                  d,
    E                  e,
    F                  f,
    G                  g,
    H                  h,
    I                  i
    )
{
    proto::encoder::null_type no;
    return new_entry(
            fe, sv, fmt, a, b, c, d, e, f, g, h, i, no, no, no, no, no
            );
}
//------------------------------------------------------------------------------
template<class A, class B, class C, class D, class E, class F, class G, class H>
bool new_entry(                                                                 //don't use this directly, use the macros!
    frontend&          fe,
    sev::severity      sv,
    proto::str_literal fmt,
    A                  a,
    B                  b,
    C                  c,
    D                  d,
    E                  e,
    F                  f,
    G                  g,
    H                  h
    )
{
    proto::encoder::null_type no;
    return new_entry(
            fe, sv, fmt, a, b, c, d, e, f, g, h, no, no, no, no, no, no
            );
}
//------------------------------------------------------------------------------
template<class A, class B, class C, class D, class E, class F, class G>
bool new_entry(                                                                 //don't use this directly, use the macros!
    frontend&          fe,
    sev::severity      sv,
    proto::str_literal fmt,
    A                  a,
    B                  b,
    C                  c,
    D                  d,
    E                  e,
    F                  f,
    G                  g
    )
{
    proto::encoder::null_type no;
    return new_entry(
            fe, sv, fmt, a, b, c, d, e, f, g, no, no, no, no, no, no, no
            );
}
//------------------------------------------------------------------------------
template<class A, class B, class C, class D, class E, class F>
bool new_entry(                                                                 //don't use this directly, use the macros!
    frontend&          fe,
    sev::severity      sv,
    proto::str_literal fmt,
    A                  a,
    B                  b,
    C                  c,
    D                  d,
    E                  e,
    F                  f
    )
{
    proto::encoder::null_type no;
    return new_entry(
            fe, sv, fmt, a, b, c, d, e, f, no, no, no, no, no, no, no, no
            );
}
//------------------------------------------------------------------------------
template<class A, class B, class C, class D, class E>
bool new_entry(                                                                 //don't use this directly, use the macros!
    frontend&          fe,
    sev::severity      sv,
    proto::str_literal fmt,
    A                  a,
    B                  b,
    C                  c,
    D                  d,
    E                  e
    )
{
    proto::encoder::null_type no;
    return new_entry(
            fe, sv, fmt, a, b, c, d, e, no, no, no, no, no, no, no, no, no
            );
}
//------------------------------------------------------------------------------
template<class A, class B, class C, class D>
bool new_entry(                                                                 //don't use this directly, use the macros!
    frontend&          fe,
    sev::severity      sv,
    proto::str_literal fmt,
    A                  a,
    B                  b,
    C                  c,
    D                  d
    )
{
    proto::encoder::null_type no;
    return new_entry(
            fe, sv, fmt, a, b, c, d, no, no, no, no, no, no, no, no, no, no
            );
}
//------------------------------------------------------------------------------
template<class A, class B, class C>
bool new_entry(
        frontend& fe, sev::severity sv, proto::str_literal fmt, A a, B b, C c
        )
{
    proto::encoder::null_type no;
    return new_entry(
        fe, sv, fmt, a, b, c, no, no, no, no, no, no, no, no, no, no, no
        );
}
//------------------------------------------------------------------------------
template<class A, class B>
bool new_entry(
        frontend& fe, sev::severity sv, proto::str_literal fmt, A a, B b
        )
{
    proto::encoder::null_type no;
    return new_entry(
        fe, sv, fmt, a, b, no, no, no, no, no, no, no, no, no, no, no, no
        );
}
//------------------------------------------------------------------------------
template<class A>
bool new_entry(
        frontend& fe, sev::severity sv, proto::str_literal fmt, A a
        )
{
    proto::encoder::null_type no;
    return new_entry(
        fe, sv, fmt, a, no, no, no, no, no, no, no, no, no, no, no, no, no
        );
}
//------------------------------------------------------------------------------
inline bool new_entry(
        frontend& fe, sev::severity sv, proto::str_literal fmt
        )
{
    proto::encoder::null_type no;
    return new_entry(
        fe, sv, fmt, no, no, no, no, no, no, no, no, no, no, no, no, no, no
        );
}
//------------------------------------------------------------------------------
} //namespace

#endif /* UFO_LOG_INTERFACE_HPP_ */
