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

#ifndef MAL_LOG_STATIC_RDBUF_HPP_
#define MAL_LOG_STATIC_RDBUF_HPP_

#include <mal_log/util/integer.hpp>
#include <mal_log/frontend_types.hpp>
#include <streambuf>

namespace mal {

//------------------------------------------------------------------------------
template <uword capacity>
class static_rdbuf : public std::basic_streambuf<char, std::char_traits<char> >
{
public:
    //--------------------------------------------------------------------------
    static_rdbuf()
    {
        reset();
    }
    //--------------------------------------------------------------------------
    virtual ~static_rdbuf() {};
    //--------------------------------------------------------------------------
    deep_copy_string get_deep_copy_string() const
    {
        deep_copy_string s;
        s.mem  = m_buff;
        s.size = pptr() - pbase();
        return s;
    }
    //--------------------------------------------------------------------------
private:
    template <uword> friend class stack_ostream;
    //--------------------------------------------------------------------------
    void reset()
    {
        setp (m_buff, m_buff + sizeof m_buff);
    };
    //--------------------------------------------------------------------------
    static_rdbuf (const static_rdbuf &);
    static_rdbuf &operator= (const static_rdbuf &);
    //--------------------------------------------------------------------------
    virtual int_type overflow (int_type ch)
    {
        return traits_type::eof();
    }
    //--------------------------------------------------------------------------
    virtual int sync()
    {
        return pptr() < epptr() ? 0 : -1;
    }
    //--------------------------------------------------------------------------
    char m_buff[capacity];
};
//------------------------------------------------------------------------------
} //namespaces

#endif /* MAL_LOG_STATIC_RDBUF_HPP_ */
