/*
--------------------------------------------------------------------------------
The code as presented here:
http://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue
is licensed by Dmitry Vyukov under the terms below:

BSD 2-clause license

Copyright (c) 2010-2011 Dmitry Vyukov. All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY DMITRY VYUKOV "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
SHALL DMITRY VYUKOV OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of Dmitry Vyukov.

--------------------------------------------------------------------------------
The code in its current form adds the license below:

BSD 3-clause license

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

#ifndef UFO_SPMC_HPP_
#define UFO_SPMC_HPP_

#include <cassert>
#include <ufo_log/util/atomic.hpp>

namespace ufo {

//------------------------------------------------------------------------------
template<typename T>
class spmc_b_fifo
{
public:
    //--------------------------------------------------------------------------
    spmc_b_fifo (size_t buffer_size) :
          m_buffer      (nullptr),
          m_buffer_mask (buffer_size - 1)
    {
        assert ((buffer_size >= 2) && ((buffer_size & (buffer_size - 1)) == 0));

        m_buffer = new cell_t [buffer_size];

        for (size_t i = 0; i != buffer_size; i += 1)
        {
            m_buffer[i].m_sequence = i;
        }

        m_enqueue_pos = 0;
        m_dequeue_pos = 0;
    }
    //--------------------------------------------------------------------------
    ~spmc_b_fifo()
    {
        if (m_buffer)
        {
            delete [] m_buffer;
        }
    }
    //--------------------------------------------------------------------------
    bool bounded_push (T const& data)
    {
        assert (m_buffer);
        cell_t* cell  = &m_buffer[m_enqueue_pos & m_buffer_mask];
        size_t seq    = cell->m_sequence.load (mo_acquire);
        intptr_t diff = (intptr_t) seq - (intptr_t) m_enqueue_pos;
        if (diff == 0)
        {
            ++m_enqueue_pos;
            cell->m_data = data;
            cell->m_sequence.store (m_enqueue_pos, mo_release);
            return true;
        }
        assert (diff < 0);
        return false;
    }
    //--------------------------------------------------------------------------
    bool pop (T& data)
    {
        assert (m_buffer);
        cell_t* cell;
        size_t pos = m_dequeue_pos;
        for (;;)
        {
            cell          = &m_buffer[pos & m_buffer_mask];
            size_t seq    = cell->m_sequence.load (mo_acquire);
            intptr_t diff = (intptr_t) seq - (intptr_t) (pos + 1);
            if (diff == 0)
            {
                if (m_dequeue_pos.compare_exchange_weak(
                    pos, pos + 1, mo_relaxed
                    ))
                {
                    break;
                }
            }
            else if (diff < 0)
            {
                return false;
            }
            else
            {
                pos = m_dequeue_pos;
            }
        }

        data = cell->m_data;
        cell->m_sequence.store (pos + m_buffer_mask + 1, mo_release);

        return true;
    }
    //--------------------------------------------------------------------------
private:
    //--------------------------------------------------------------------------
    struct cell_t
    {
        at::atomic<size_t> m_sequence;
        T                  m_data;
    };
    //--------------------------------------------------------------------------
    typedef char cacheline_pad_t [cache_line_size];

    cacheline_pad_t           m_pad0;

    cell_t*                   m_buffer;
    size_t const              m_buffer_mask;

    cacheline_pad_t           m_pad1;

    mo_relaxed_atomic<size_t> m_enqueue_pos;

    cacheline_pad_t           m_pad2;

    mo_relaxed_atomic<size_t> m_dequeue_pos;

    cacheline_pad_t           m_pad3;

    spmc_b_fifo (spmc_b_fifo const&);
    void operator= (spmc_b_fifo const&);
};
//------------------------------------------------------------------------------
} //namespaces

#endif /* SPMC_HPP_ */
