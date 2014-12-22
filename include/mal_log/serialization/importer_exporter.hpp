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

#ifndef MAL_LOG_IMPORTER_EXPORTER_BASE_HPP_
#define MAL_LOG_IMPORTER_EXPORTER_BASE_HPP_

#include <cassert>
#include <mal_log/util/integer.hpp>

namespace mal { namespace ser {

//------------------------------------------------------------------------------
template <class ptr_type>
class importer_exporter                                                         //could be named vandelay too.. https://www.youtube.com/watch?v=67L0pbneT2w
{
public:
    //--------------------------------------------------------------------------
    struct null_type {};
    //--------------------------------------------------------------------------
    importer_exporter()
    {
        zero();
    }
    //--------------------------------------------------------------------------
    void init (ptr_type* mem, uword msg_total_size)
    {
        assert (mem);
        assert (msg_total_size);

        m_pos = m_beg = mem;
        m_end = m_pos + msg_total_size;
    }
    //--------------------------------------------------------------------------
    void init (ptr_type* mem)
    {
        assert (mem);
        uword total_size = ((uword) -1) - ((uword) mem);
        init (mem, total_size);
    }
    //--------------------------------------------------------------------------
    bool has_memory() const
    {
        return m_pos != nullptr;
    }
    //--------------------------------------------------------------------------
    u8* get_memory() const
    {
        return m_beg;
    }
    //--------------------------------------------------------------------------
protected:

    //--------------------------------------------------------------------------
    void zero()
    {
        m_beg = m_pos = m_end = nullptr;
    }
    //--------------------------------------------------------------------------
    ptr_type* m_pos;
    ptr_type* m_beg;
    ptr_type* m_end;
    //--------------------------------------------------------------------------
}; //class encoder_decoder_base
//------------------------------------------------------------------------------

}} //namespaces

#endif /* MAL_LOG_IMPORTER_EXPORTER_BASE_HPP_ */
