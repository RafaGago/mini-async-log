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

#ifndef MAL_ALIGNED_TYPE_HPP_
#define MAL_ALIGNED_TYPE_HPP_

#include <mal_log/util/placement_new.hpp>

//------------------------------------------------------------------------------
namespace mal {
//------------------------------------------------------------------------------
template <class T, std::size_t alignment = std::alignment_of<T>::value>
class aligned_type
{
public:
    //--------------------------------------------------------------------------
    ~aligned_type() { m_mem.destruct(); };
    //--------------------------------------------------------------------------
    T& get()                    { return m_mem.get(); };
    const T& get() const        { return m_mem.get(); };
    T& operator*()              { return m_mem.operator*(); };
    const T& operator*() const  { return m_mem.operator*(); };
    T* operator->()             { return m_mem.operator->(); };
    const T* operator->() const { return m_mem.operator->(); };
    //--------------------------------------------------------------------------
#ifdef MAL_HAS_VARIADIC_TEMPLATES
    //--------------------------------------------------------------------------
    template <class... args>
    aligned_type (args&&... a)
    {
        m_mem.construct (std::forward<args>(a)...);
    }
#else
    //--------------------------------------------------------------------------
    aligned_type()
    {
        m_mem.construct();
    };
    //--------------------------------------------------------------------------
    template<class A>
    aligned_type (A&& a)
    {
        using namespace std;
        m_mem.construct (forward<A> (a));
    };
    //--------------------------------------------------------------------------
    template<class A, class B>
    aligned_type (A&& a, B&& b)
    {
        using namespace std;
        m_mem.construct (forward<A> (a), forward<B> (b));
    };
    //--------------------------------------------------------------------------
    template<class A, class B, class C, class D>
    aligned_type (A&& a, B&& b, C&& c, D&& d)
    {
        using namespace std;
        m_mem.construct(
            forward<A> (a), forward<B> (b), forward<C> (c), forward<D> (d)
            );
    };
    //--------------------------------------------------------------------------
    template<class A, class B, class C, class D, class E>
    aligned_type (A&& a, B&& b, C&& c, D&& d, E&& e)
    {
        using namespace std;
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
    aligned_type (A&& a, B&& b, C&& c, D&& d, E&& e, F&& f)
    {
        using namespace std;
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
    aligned_type (A&& a, B&& b, C&& c, D&& d, E&& e, F&& f, G&& g)
    {
        using namespace std;
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
    aligned_type (A&& a, B&& b, C&& c, D&& d, E&& e, F&& f, G&& g, H&& h)
    {
        using namespace std;
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
    aligned_type(
        A&& a, B&& b, C&& c, D&& d, E&& e, F&& f, G&& g, H&& h, I&& i
        )
    {
        using namespace std;
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
    aligned_type(
        A&& a, B&& b, C&& c, D&& d, E&& e, F&& f, G&& g, H&& h, I&& i, J&& j
        )
    {
        using namespace std;
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
    placement_new<T, alignment> m_mem;
};
//------------------------------------------------------------------------------
} //namespaces

#endif /* ALIGNED_TYPE_HPP_ */
