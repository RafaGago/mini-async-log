/*
The BSD 3-clause license
--------------------------------------------------------------------------------
Copyright (c) 2014 Rafael Gago Castano. All rights reserved.

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

THIS SOFTWARE IS PROVIDED BY RAFAEL GAGO CASTANO "AS IS" AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
SHALL RAFAEL GAGO CASTANO OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of Rafael Gago Castano.
--------------------------------------------------------------------------------
*/

#ifndef MAL_LOG_STACK_OSTREAM_HPP_
#define MAL_LOG_STACK_OSTREAM_HPP_

#include <mal_log/util/static_rdbuf.hpp>
#include <mal_log/frontend_types.hpp>
#include <ostream>
#include <type_traits>

namespace mal {

//------------------------------------------------------------------------------
template <uword capacity>
class stack_ostream : public std::basic_ostream<char, std::char_traits<char> >
{
public:
    //--------------------------------------------------------------------------
    stack_ostream() :
        std::basic_ostream<char, std::char_traits<char> > (&m_buff)
    {}
    //--------------------------------------------------------------------------
    deep_copy_string get_deep_copy_string() const
    {
        return m_buff.get_deep_copy_string();
    }
    //--------------------------------------------------------------------------
    void reuse()
    {
        clear();
        m_buff.reset();
    }
    //--------------------------------------------------------------------------
private:
    static_rdbuf<capacity> m_buff;
    //--------------------------------------------------------------------------
};
//------------------------------------------------------------------------------

namespace macro {
//------------------------------------------------------------------------------
template <uword capacity>
inline stack_ostream<capacity>&
you_can_just_use_stack_ostream_with_ostr_deep_copy(
        stack_ostream<capacity>& v
        )
{
    return v;
}

//------------------------------------------------------------------------------
}} //namespaces

//An ugly (very ugly actually) dangerous and pure bruteforceish hack. This is
//just meant to be used as a last resort with classes that doesn't expose
//printable members but they overload the ostream operator<<, e.g. boost::asio
//endpoints.
//
//If you know a better way to do it that it's stack based and can be lazy eva-
//luated please, don't doubt to share.

#define ostr_deep_copy(var, expr)\
    ((typename std::remove_reference<decltype(var)>::type&)(\
        ::mal::macro::you_can_just_use_stack_ostream_with_ostr_deep_copy (var)\
        << expr\
        )).get_deep_copy_string()

#endif /* MAL_LOG_STACK_OSTREAM_HPP_ */
