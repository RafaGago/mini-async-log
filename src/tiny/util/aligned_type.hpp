/*
 * aligned_type.hpp
 *
 *  Created on: Oct 17, 2013
 *      Author: rafgag
 */

#ifndef TINY_ALIGNED_TYPE_HPP_
#define TINY_ALIGNED_TYPE_HPP_

#include <tiny/util/placement_new.hpp>

//------------------------------------------------------------------------------
namespace tiny {
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
#ifdef TINY_HAS_VARIADIC_TEMPLATES
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
