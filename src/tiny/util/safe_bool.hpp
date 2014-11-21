/*
 * safe_bool.hpp
 *
 *  Created on: Mar 16, 2013
 *      Author: rafa
 */

#ifndef TINY_SAFE_BOOL_HPP_
#define TINY_SAFE_BOOL_HPP_

namespace tiny {
//------------------------------------------------------------------------------
template <typename T>
struct safe_bool_impl
{
    typedef T*    T_ptr;
    typedef T_ptr safe_bool_impl::*bool_type;
    T_ptr         stub;
};
//------------------------------------------------------------------------------
template <typename derived>
struct safe_bool
{
private:
    typedef safe_bool_impl<derived>               my_safe_bool_impl;
    typedef typename my_safe_bool_impl::bool_type bool_type;

public:
    //--------------------------------------------------------------------------
    operator bool_type() const
    {
        return static_cast<const derived*> (this)->operator_bool() ?
            &my_safe_bool_impl::stub : 0;
    }
    //--------------------------------------------------------------------------
    operator bool_type()
    {
        return static_cast<derived*> (this)->operator_bool() ?
            &my_safe_bool_impl::stub : 0;
    }
    //--------------------------------------------------------------------------
}; //class safe_bool
//------------------------------------------------------------------------------
} //namespace tiny

#endif /* TINY_SAFE_BOOL_HPP_ */

