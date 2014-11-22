/*
 * on_stack_dynamic.hpp
 *
 *  Created on: Nov 20, 2014
 *      Author: rafgag
 */


#ifndef TINY_ON_STACK_DYNAMIC_HPP_
#define TINY_ON_STACK_DYNAMIC_HPP_

#include <utility>
#include <cassert>
#include <tiny/util/placement_new.hpp>
#include <tiny/util/safe_bool.hpp>

//------------------------------------------------------------------------------
namespace tiny {
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
        if (m_constructed)
        {
            m_mem.destruct();
        }
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
    void destruct()
    {
        if (m_constructed)
        {
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
#ifdef TINY_HAS_VARIADIC_TEMPLATES
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
    placement_new<T, alignment> m_mem;
    bool                        m_constructed;
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
        if (m_destruct)
        {
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

#endif /* TINY_ON_STACK_DYNAMIC_HPP_ */
