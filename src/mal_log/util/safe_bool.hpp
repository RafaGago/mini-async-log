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

#ifndef MAL_SAFE_BOOL_HPP_
#define MAL_SAFE_BOOL_HPP_

namespace mal {
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
} //namespace mal

#endif /* MAL_SAFE_BOOL_HPP_ */

