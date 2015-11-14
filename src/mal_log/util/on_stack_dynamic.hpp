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

#ifndef MAL_ON_STACK_DYNAMIC_HPP_
#define MAL_ON_STACK_DYNAMIC_HPP_

#include <utility>
#include <cassert>
#include <mal_log/util/placement_new.hpp>
#include <mal_log/util/safe_bool.hpp>

//------------------------------------------------------------------------------
namespace mal {
//------------------------------------------------------------------------------
template <class T, std::size_t align>
class on_stack_dynamic_scoped_destructor;
//------------------------------------------------------------------------------
template <class T, std::size_t alignment = std::alignment_of<T>::value>
class on_stack_dynamic :
        public safe_bool<on_stack_dynamic<T, alignment> >
{
public:
    //--------------------------------------------------------------------------
    typedef on_stack_dynamic_scoped_destructor<T, alignment> dynamic_destructor;
    //--------------------------------------------------------------------------
    on_stack_dynamic()
    {
        m_constructed = false;
    }
    //--------------------------------------------------------------------------
    ~on_stack_dynamic()
    {
        destruct_if();
    };
    //--------------------------------------------------------------------------
    T& get()
    {
        assert (m_constructed);
        return m_mem.get();
    };
    //--------------------------------------------------------------------------
    const T& get() const
    {
        assert (m_constructed);
        return m_mem.get();
    };
    //--------------------------------------------------------------------------
    T& operator*()
    {
        assert (m_constructed);
        return m_mem.operator*();
    };
    //--------------------------------------------------------------------------
    const T& operator*() const
    {
        assert (m_constructed);
        return m_mem.operator*();
    };
    //--------------------------------------------------------------------------
    T* operator->()
    {
        assert (m_constructed);
        return m_mem.operator->();
    };
    //--------------------------------------------------------------------------
    const T* operator->() const
    {
        assert (m_constructed);
        return m_mem.operator->();
    };
    //--------------------------------------------------------------------------
    void destruct_if()
    {
        if (m_constructed) {
            m_mem.destruct();
            m_constructed = false;
        }
    }
    //--------------------------------------------------------------------------
    bool is_constructed() const
    {
        return m_constructed;
    }
    //--------------------------------------------------------------------------
    bool operator_bool() const
    {
        return is_constructed();
    }
    //--------------------------------------------------------------------------
#ifdef MAL_HAS_VARIADIC_TEMPLATES
    //--------------------------------------------------------------------------
    template <class... args>
    void construct (args&&... a)
    {
        assert (!m_constructed);
        m_constructed = true;
        m_mem.construct (std::forward<args>(a)...);
    }
    //--------------------------------------------------------------------------
#else
    //--------------------------------------------------------------------------
    void construct()
    {
        assert (!m_constructed);
        m_constructed = true;
        m_mem.construct();
    };
    //--------------------------------------------------------------------------
    template<class A>
    void construct (A&& a)
    {
        using namespace std;
        assert (!m_constructed);
        m_constructed = true;
        m_mem.construct (forward<A> (a));
    };
    //--------------------------------------------------------------------------
    template<class A, class B>
    void construct (A&& a, B&& b)
    {
        using namespace std;
        assert (!m_constructed);
        m_constructed = true;
        m_mem.construct (forward<A> (a), forward<B> (b));
    };
    //--------------------------------------------------------------------------
    template<class A, class B, class C, class D>
    void construct (A&& a, B&& b, C&& c, D&& d)
    {
        using namespace std;
        assert (!m_constructed);
        m_constructed = true;
        m_mem.construct(
            forward<A> (a), forward<B> (b), forward<C> (c), forward<D> (d)
            );
    };
    //--------------------------------------------------------------------------
    template<class A, class B, class C, class D, class E>
    void construct (A&& a, B&& b, C&& c, D&& d, E&& e)
    {
        using namespace std;
        assert (!m_constructed);
        m_constructed = true;
        m_mem.construct(
            forward<A> (a),
            forward<B> (b),
            forward<C> (c),
            forward<D> (d),
            forward<E> (e)
            );
    };
    //--------------------------------------------------------------------------
    template<class A, class B, class C, class D, class E, class F>
    void construct (A&& a, B&& b, C&& c, D&& d, E&& e, F&& f)
    {
        using namespace std;
        assert (!m_constructed);
        m_constructed = true;
        m_mem.construct(
            forward<A> (a),
            forward<B> (b),
            forward<C> (c),
            forward<D> (d),
            forward<E> (e),
            forward<F> (f),
            );
    };
    //--------------------------------------------------------------------------
    template<class A, class B, class C, class D, class E, class F, class G>
    void construct (A&& a, B&& b, C&& c, D&& d, E&& e, F&& f, G&& g)
    {
        using namespace std;
        assert (!m_constructed);
        m_constructed = true;
        m_mem.construct(
            forward<A> (a),
            forward<B> (b),
            forward<C> (c),
            forward<D> (d),
            forward<E> (e),
            forward<F> (f),
            forward<G> (g)
            );
    };
    //--------------------------------------------------------------------------
    template<
        class A, class B, class C, class D, class E, class F, class G, class H
        >
    void construct (A&& a, B&& b, C&& c, D&& d, E&& e, F&& f, G&& g, H&& h)
    {
        using namespace std;
        assert (!m_constructed);
        m_constructed = true;
        m_mem.construct(
            forward<A> (a),
            forward<B> (b),
            forward<C> (c),
            forward<D> (d),
            forward<E> (e),
            forward<F> (f),
            forward<G> (g),
            forward<H> (h)
            );
    };
    //--------------------------------------------------------------------------
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
    void construct(
        A&& a, B&& b, C&& c, D&& d, E&& e, F&& f, G&& g, H&& h, I&& i
        )
    {
        using namespace std;
        assert (!m_constructed);
        m_constructed = true;
        m_mem.construct(
            forward<A> (a),
            forward<B> (b),
            forward<C> (c),
            forward<D> (d),
            forward<E> (e),
            forward<F> (f),
            forward<G> (g),
            forward<H> (h),
            forward<I> (i)
            );
    };
    //--------------------------------------------------------------------------
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
    void construct(
        A&& a, B&& b, C&& c, D&& d, E&& e, F&& f, G&& g, H&& h, I&& i, J&& j
        )
    {
        using namespace std;
        assert (!m_constructed);
        m_constructed = true;
        m_mem.construct(
            forward<A> (a),
            forward<B> (b),
            forward<C> (c),
            forward<D> (d),
            forward<E> (e),
            forward<F> (f),
            forward<G> (g),
            forward<H> (h),
            forward<I> (i),
            forward<J> (j)
            );
    };
    //--------------------------------------------------------------------------
#endif

private:
    bool                        m_constructed;
    placement_new<T, alignment> m_mem;
};
//------------------------------------------------------------------------------
template <class T, std::size_t align>
on_stack_dynamic_scoped_destructor<T, align>
    get_scoped_destructor (on_stack_dynamic<T, align>& ref);
//------------------------------------------------------------------------------
template <class T, std::size_t align = std::alignment_of<T>::value>
class on_stack_dynamic_scoped_destructor
{
public:
    //--------------------------------------------------------------------------
    typedef on_stack_dynamic<T, align>                   ref_type;
    typedef on_stack_dynamic_scoped_destructor<T, align> my_type;
    //--------------------------------------------------------------------------
    on_stack_dynamic_scoped_destructor (my_type&& other)
    {
        *this = std::forward<my_type> (other);
    }
    //--------------------------------------------------------------------------
    my_type& operator= (my_type&& other)
    {
        m_destruct       = other.m_destruct;
        other.m_destruct = nullptr;
        return *this;
    }
    //--------------------------------------------------------------------------
    on_stack_dynamic_scoped_destructor (ref_type& ref)
    {
        m_destruct = &ref;
    }
    //--------------------------------------------------------------------------
    ~on_stack_dynamic_scoped_destructor()
    {
        if (m_destruct) {
            m_destruct->destruct();
        }
    }
    //--------------------------------------------------------------------------
    void cancel()
    {
        m_destruct = nullptr;
    }
    //--------------------------------------------------------------------------
private:
    //--------------------------------------------------------------------------
    friend my_type get_scoped_destructor<T, align> (ref_type& ref);
    //--------------------------------------------------------------------------
    ref_type* m_destruct;
    //--------------------------------------------------------------------------
};
//------------------------------------------------------------------------------
template <class T, std::size_t align>
on_stack_dynamic_scoped_destructor<T, align>
        get_scoped_destructor (on_stack_dynamic<T, align>& ref)
{
    return on_stack_dynamic_scoped_destructor<T, align> (ref);
}
//------------------------------------------------------------------------------
} //namespaces

#endif /* MAL_ON_STACK_DYNAMIC_HPP_ */
