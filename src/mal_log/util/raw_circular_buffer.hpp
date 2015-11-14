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

#ifndef MAL_LOG_RAW_CIRCULAR_BUFFER_HPP_
#define MAL_LOG_RAW_CIRCULAR_BUFFER_HPP_

#include <cassert>
#include <new>
#include <mal_log/util/integer.hpp>

namespace mal {
//------------------------------------------------------------------------------
// Simple circular queue containing a single chunk of raw non-guarateed to be
// aligned memory
//------------------------------------------------------------------------------
class raw_circular_buffer
{
public:
    //--------------------------------------------------------------------------
    raw_circular_buffer()
    {
        m_mem = nullptr;
        free();
    }
    //--------------------------------------------------------------------------
    ~raw_circular_buffer()
    {
        free();
    }
    //--------------------------------------------------------------------------
    bool init (uword elem_size, uword elem_count)
    {
        assert (!is_initialized());
        if (elem_size && elem_count) {
            m_entry_size = elem_size;
            m_total_size = elem_size * elem_count;
            if ((m_total_size / m_entry_size) == elem_count) {
                return allocate();
            }
        }
        return false;
    }
    //--------------------------------------------------------------------------
    void free()
    {
        if (m_mem) {
            ::operator delete (m_mem);
        }
        m_size = m_entry_size = m_total_size = 0;
        m_mem = m_tail = nullptr;
    }
    //--------------------------------------------------------------------------
    bool is_initialized() const { return m_mem != nullptr; }
    //--------------------------------------------------------------------------
    bool is_empty() const { return byte_size() == 0; }
    //--------------------------------------------------------------------------
    bool is_full() const { return byte_size() == byte_capacity(); }
    //--------------------------------------------------------------------------
    uword byte_size() const   { return m_size; }
    //--------------------------------------------------------------------------
    uword entry_byte_size() const { return m_entry_size; }
    //--------------------------------------------------------------------------
    uword byte_capacity() const { return m_total_size; }
    //--------------------------------------------------------------------------
    uword size() const   { return byte_size() / entry_byte_size(); }
    //--------------------------------------------------------------------------
    uword capacity() const { return byte_capacity() / entry_byte_size(); }
    //--------------------------------------------------------------------------
    void* head()
    {
        assert (!is_empty());
        uword tail_pos = m_tail - m_mem;
        uword offset   = m_size - m_entry_size;
        uword head_pos = (tail_pos >= offset) ?
                (tail_pos - m_size + m_entry_size) :
                (tail_pos + m_total_size - offset);
        return (void*) (m_mem + head_pos);
    }
    //--------------------------------------------------------------------------
    void pop_head()
    {
        assert (!is_empty());
        m_size -= m_entry_size;
    }
    //--------------------------------------------------------------------------
    void* tail()
    {
        assert (!is_empty());
        return (void*) m_tail;
    }
    //--------------------------------------------------------------------------
    void push_tail()
    {
        assert (!is_full());
        m_size += m_entry_size;
        m_tail += m_entry_size;
        m_tail  = (m_tail < (m_mem + m_total_size)) ? m_tail : m_mem;
    }
    //--------------------------------------------------------------------------
private:
    //--------------------------------------------------------------------------
    bool allocate()
    {
        m_mem = (u8*) ::operator new (m_total_size, std::nothrow);
        if (m_mem) {
            m_tail = m_mem + m_total_size;
            m_size = 0;
            return true;
        }
        return false;
    }
    //--------------------------------------------------------------------------
    uword m_size, m_entry_size, m_total_size;
    u8 *m_mem, *m_tail;
    //--------------------------------------------------------------------------
};
//------------------------------------------------------------------------------
} //namespaces

#endif /* MAL_LOG_RAW_CIRCULAR_BUFFER_HPP_ */
