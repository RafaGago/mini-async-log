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

#ifndef MAL_PLACEMENT_NEW_HPP_
#define MAL_PLACEMENT_NEW_HPP_

#include <type_traits>
#include <utility>
#include <mal_log/util/system.hpp>

//------------------------------------------------------------------------------
namespace mal {
//------------------------------------------------------------------------------
template <class T, std::size_t alignment = std::alignment_of<T>::value>
class placement_new
{
public:
    //--------------------------------------------------------------------------
    placement_new () {};
    //--------------------------------------------------------------------------
    T& get()                    { return *((T*) &m_stor); }
    const T& get() const        { return *((T*) &m_stor); }
    T& operator*()              { return get(); }
    const T& operator*() const  { return get(); }
    T* operator->()             { return ((T*) &m_stor); }
    const T* operator->() const { return ((T*) &m_stor); }
    void destruct()             { get().~T(); }
    //--------------------------------------------------------------------------
#ifdef MAL_HAS_VARIADIC_TEMPLATES
    template <class... args>
    void construct (args&&... a)
    {
        new (&m_stor) T (std::forward<args>(a)...);
    }
    //--------------------------------------------------------------------------
#else
    //--------------------------------------------------------------------------
    void construct()
    {
        new (&m_stor) T();
    };
    //--------------------------------------------------------------------------
    template<class A>
    void construct (A&& a)
    {
        using namespace std;
        new (&m_stor) T (forward<A> (a));
    };
    //--------------------------------------------------------------------------
    template<class A, class B>
    void construct (A&& a, B&& b)
    {
        using namespace std;
        new (&m_stor) T (forward<A> (a), forward<B> (b));
    };
    //--------------------------------------------------------------------------
    template<class A, class B, class C, class D>
    void construct (A&& a, B&& b, C&& c, D&& d)
    {
        using namespace std;
        new (&m_stor) T(
            forward<A> (a), forward<B> (b), forward<C> (c), forward<D> (d)
            );
    };
    //--------------------------------------------------------------------------
    template<class A, class B, class C, class D, class E>
    void construct (A&& a, B&& b, C&& c, D&& d, E&& e)
    {
        using namespace std;
        new (&m_stor) T(
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
        new (&m_stor) T(
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
        new (&m_stor) T(
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
        new (&m_stor) T(
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
        new (&m_stor) T(
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
        new (&m_stor) T(
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
    placement_new (const placement_new& other);
    placement_new& operator= (const placement_new& other);

    typename std::aligned_storage<sizeof (T), alignment>::type m_stor;

}; //class aligned_type

} //namespaces

#endif /* MAL_SERVER_PLACEMENT_NEW_HPP_ */
