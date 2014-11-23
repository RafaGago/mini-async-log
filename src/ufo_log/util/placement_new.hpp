/*
 * placement_new.hpp
 *
 *  Created on: Nov 17, 2014
 *      Author: rafgag
 */


#ifndef UFO_PLACEMENT_NEW_HPP_
#define UFO_PLACEMENT_NEW_HPP_

#include <type_traits>
#include <utility>
#include <ufo_log/util/system.hpp>

//------------------------------------------------------------------------------
namespace ufo {
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
#ifdef UFO_HAS_VARIADIC_TEMPLATES
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

#endif /* UFO_SERVER_PLACEMENT_NEW_HPP_ */
