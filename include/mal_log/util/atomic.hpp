/*
The BSD 3-clause license

Copyright (c) 2013-2014 Diadrom AB. All rights reserved.

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

THIS SOFTWARE IS PROVIDED BY DIADROM AB "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
SHALL DIADROM AB OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of Diadrom AB.
--------------------------------------------------------------------------------
*/

#ifndef MAL_ATOMIC_HPP_
#define MAL_ATOMIC_HPP_

#include <mal_log/util/integer.hpp>

#ifdef MAL_USE_BOOST_ATOMIC
    #include <boost/atomic.hpp>
    #define MAL_ATOMIC_NAMESPACE ::boost

#else
    #include <atomic>
    #define MAL_ATOMIC_NAMESPACE ::std

#endif

namespace mal {

namespace at = MAL_ATOMIC_NAMESPACE;

typedef at::atomic<u8>     atomic_uint8;
typedef at::atomic<u16>    atomic_uint16;
typedef at::atomic<u32>    atomic_uint32;
typedef at::atomic<u64>    atomic_uint64;

typedef at::atomic<u8>     atomic_u8;
typedef at::atomic<u16>    atomic_u16;
typedef at::atomic<u32>    atomic_u32;
typedef at::atomic<u64>    atomic_u64;

typedef at::atomic<i8>     atomic_int8;
typedef at::atomic<i16>    atomic_int16;
typedef at::atomic<i32>    atomic_int32;
typedef at::atomic<i64>    atomic_int64;

typedef at::atomic<i8>     atomic_i8;
typedef at::atomic<i16>    atomic_i16;
typedef at::atomic<i32>    atomic_i32;
typedef at::atomic<i64>    atomic_i64;

typedef at::atomic<bool>   atomic_bool;
typedef at::atomic<word>   atomic_word;
typedef at::atomic<uword>  atomic_uword;

typedef at::atomic<u8*>    atomic_uint8_ptr;
typedef at::atomic<u16*>   atomic_uint16_ptr;
typedef at::atomic<u32*>   atomic_uint32_ptr;
typedef at::atomic<u64*>   atomic_uint64_ptr;

typedef at::atomic<u8*>    atomic_u8p;
typedef at::atomic<u16*>   atomic_u16p;
typedef at::atomic<u32*>   atomic_u32p;
typedef at::atomic<u64*>   atomic_u64p;

typedef at::atomic<i8*>    atomic_int8_ptr;
typedef at::atomic<i16*>   atomic_int16_ptr;
typedef at::atomic<i32*>   atomic_int32_ptr;
typedef at::atomic<i64*>   atomic_int64_ptr;

typedef at::atomic<i8*>    atomic_i8p;
typedef at::atomic<i16*>   atomic_i16p;
typedef at::atomic<i32*>   atomic_i32p;
typedef at::atomic<i64*>   atomic_i64p;

typedef at::atomic<void*>  atomic_void_ptr;
typedef at::atomic<bool*>  atomic_bool_ptr;
typedef at::atomic<word*>  atomic_word_ptr;
typedef at::atomic<uword*> atomic_uword_ptr;

const at::memory_order memory_order_relaxed = at::memory_order_relaxed;
const at::memory_order memory_order_consume = at::memory_order_consume;
const at::memory_order memory_order_acquire = at::memory_order_acquire;
const at::memory_order memory_order_release = at::memory_order_release;
const at::memory_order memory_order_acq_rel = at::memory_order_acq_rel;
const at::memory_order memory_order_seq_cst = at::memory_order_seq_cst;

const at::memory_order mo_relaxed = memory_order_relaxed;
const at::memory_order mo_consume = memory_order_consume;
const at::memory_order mo_acquire = memory_order_acquire;
const at::memory_order mo_release = memory_order_release;
const at::memory_order mo_acq_rel = memory_order_acq_rel;
const at::memory_order mo_seq_cst = memory_order_seq_cst;

//------------------------------------------------------------------------------
// I don't agree with the committe at setting the memory order to "seq_cst" by
// default.
//
// Atomics will be often used as an idiom to say that a built-in type
// is to be accessed by many threads (even tough in most platforms built-in
// types are atomic by nature). Having an atomic doesn't necessary imply that
// one needs to synchronize and throw fences everywere with each load and store.
//
// The other major use of atomics is when people are trying to avoid OS
// threading primitives, writing low level queues, etc.
//
// I don't think that both of this cases need to default to "seq_cst" when
// load and storing.
//
// C++ is the language that should let people shoot themselves in the feet.
//------------------------------------------------------------------------------
template <class T>
class mo_relaxed_atomic : public at::atomic<T>
{
public:
    typedef typename at::atomic<T> base;

    mo_relaxed_atomic() : base() {};
    mo_relaxed_atomic (T val) : base (val) {};

    operator T() const          { return this->load (mo_relaxed); }
    operator T() const volatile { return this->load (mo_relaxed); }

    T val() const               { return (T) *this;};
    T val() const volatile      { return (T) *this;};

    T operator= (T v)           { this->store (v, mo_relaxed); return v; }
    T operator= (T v) volatile  { this->store (v, mo_relaxed); return v; }

};
//------------------------------------------------------------------------------
template <class T>
class mo_relaxed_atomic<T*> : public at::atomic<T*>
{
public:
    typedef T* ptr_type;
    typedef typename at::atomic<ptr_type> base;

    mo_relaxed_atomic() : base() {};
    mo_relaxed_atomic (ptr_type val) : base (val) {};

    inline operator ptr_type() const
    {
        return this->load (mo_relaxed);
    }

    inline operator ptr_type() const volatile
    {
        return this->load (mo_relaxed);
    }

    ptr_type val() const
    {
        return (ptr_type) *this;

    };
    ptr_type val() const volatile
    {
        return (ptr_type) *this;
    };

    inline ptr_type operator= (ptr_type v)
    {
        this->store (v, mo_relaxed); return v;
    }

    inline ptr_type operator= (ptr_type v) volatile
    {
        this->store (v, mo_relaxed); return v;
    }

    inline T& operator*()
    {
        return *this->load (mo_relaxed);
    }

    inline const T& operator*() const
    {
        return *this->load (mo_relaxed);
    }

    inline ptr_type operator->()
    {
        return this->load (mo_relaxed);
    }

    inline const ptr_type operator->() const
    {
        return this->load (mo_relaxed);
    }

    inline ptr_type operator&() const
    {
        return this->load (mo_relaxed);
    }
};
//------------------------------------------------------------------------------
template <> //todo remove duplications, now I'm in a rush...
class mo_relaxed_atomic<void*> : public at::atomic<void*>
{
public:
    typedef void* ptr_type;
    typedef at::atomic<ptr_type> base;

    mo_relaxed_atomic() : base() {};
    mo_relaxed_atomic (ptr_type val) : base (val) {};

    inline operator ptr_type() const
    {
        return this->load (mo_relaxed);
    }

    inline operator ptr_type() const volatile
    {
        return this->load (mo_relaxed);
    }

    ptr_type val() const
    {
        return (ptr_type) *this;
    };

    ptr_type val() const volatile
    {
        return (ptr_type) *this;
    };

    inline ptr_type operator= (ptr_type v)
    {
        this->store (v, mo_relaxed);
        return v;
    }

    inline ptr_type operator= (ptr_type v) volatile
    {
        this->store (v, mo_relaxed);
        return v;
    }

    inline ptr_type operator&() const
    {
        return this->load (mo_relaxed);
    }
};
//------------------------------------------------------------------------------
} //namespace

#endif /* MAL_ATOMIC_HPP_ */
