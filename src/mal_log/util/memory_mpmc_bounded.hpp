/*
--------------------------------------------------------------------------------
The code as presented here:
http://www.1024cores.net/home/lock-free-algorithms/queues/intrusive-mpsc-node-based-queue
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

The BSD 3-clause license

Copyright (c) 2013-2014 Rafael Gago Castano. All rights reserved.

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

#if THIS_IS_UNUSED

#ifndef MAL_LOG_MEMORY_MPMC_BOUNDED_HPP_
#define MAL_LOG_MEMORY_MPMC_BOUNDED_HPP_

#include <cassert>
#include <new>
#include <stddef.h>
#include <mal_log/util/system.hpp>
#include <mal_log/util/atomic.hpp>

namespace mal {

// This is the Djukov MPMC queue that adds the trivial conversion to single
// producer or single consumer functions so it can be used as a mpmc, spmc or
// mpsc. Of course you can't mix two different producer modes on the same queue.
//
// todo: if this is to be used some day move both buffer to be a contiguous
// chunk for better memory locality.
//--------------------------------------------------------------------------
class mem_mpmc_b_prepared
{
public:
    //----------------------------------------------------------------------
    mem_mpmc_b_prepared()
    {
        cell = mem = nullptr;
        size = pos = 0;
    }
    //----------------------------------------------------------------------
    u8*    mem;
    size_t size;
    //----------------------------------------------------------------------
private:
    //----------------------------------------------------------------------
    friend class mem_mpmc_b;
    void*  cell;
    size_t pos;
    //----------------------------------------------------------------------
};
//------------------------------------------------------------------------------
class mem_mpmc_b
{
public:
    //--------------------------------------------------------------------------
    mem_mpmc_b()
    {
        m_buffer      = nullptr;
        m_mem         = nullptr;
        m_buffer_mask = 0;
        m_entry_size  = 0;
        m_enqueue_pos = 0;
        m_dequeue_pos = 0;
    }
    //--------------------------------------------------------------------------
    ~mem_mpmc_b()
    {
        clear();
    }
    //--------------------------------------------------------------------------
    void clear()                                                                //Dangerous, just to be used after failed initializations
    {
        if (m_buffer) {
            delete [] m_buffer;
        }
        if (m_mem) {
            ::operator delete (m_mem);
        }
    }
    //--------------------------------------------------------------------------
    bool init (size_t total_bytes, size_t entries)
    {
        if ((entries >= 2) &&
            ((entries & (entries - 1)) == 0) &&
            (total_bytes >= entries) &&
            !initialized()
            ) {
            clear();
            auto real_bytes = (total_bytes / entries) * entries;

            m_entry_size  = real_bytes / entries;
            m_enqueue_pos = 0;
            m_dequeue_pos = 0;
            m_buffer_mask = entries - 1;

            m_buffer = new (std::nothrow) cell_t [entries];
            if (!m_buffer) { return false; }

            m_mem = (u8*) ::operator new (real_bytes, std::nothrow);
            if (!m_mem) { return false; }

            for (size_t i = 0; i != entries; i += 1) {
                m_buffer[i].sequence = i;
                m_buffer[i].mem      = m_mem + (i * m_entry_size);
            }
            return true;
        }
        return false;
    }
    //--------------------------------------------------------------------------
    bool initialized() const
    {
        return m_buffer && m_mem;
    }
    //--------------------------------------------------------------------------
    size_t entry_size() const { return m_entry_size; }
    //--------------------------------------------------------------------------
    mem_mpmc_b_prepared mp_bounded_push_prepare()
    {
        assert (m_buffer);
        mem_mpmc_b_prepared pp;
        cell_t* cell;
        size_t pos = m_enqueue_pos;
        while (true) {
            cell          = &m_buffer[pos & m_buffer_mask];
            size_t seq    = cell->sequence.load (mo_acquire);
            intptr_t diff = (intptr_t) seq - (intptr_t) pos;
            if (diff == 0) {
                if (m_enqueue_pos.compare_exchange_weak(
                    pos, pos + 1, mo_relaxed
                    )) {
                    break;
                }
            }
            else if (diff < 0) {
                return pp;
            }
            else {
                pos = m_enqueue_pos;
            }
        }
        pp.cell = cell;
        pp.pos  = pos + 1;
        pp.mem  = (u8*) cell->mem;
        pp.size = entry_size();
        return pp;
    }
    //--------------------------------------------------------------------------
    mem_mpmc_b_prepared sp_bounded_push_prepare()
    {
        assert (m_buffer);
        mem_mpmc_b_prepared pp;
        cell_t* cell  = &m_buffer[m_enqueue_pos & m_buffer_mask];
        size_t seq    = cell->sequence.load (mo_acquire);
        intptr_t diff = (intptr_t) seq - (intptr_t) m_enqueue_pos;
        if (diff == 0) {
            ++m_enqueue_pos;
            pp.cell = cell;
            pp.pos  = m_enqueue_pos;
            pp.mem  = (u8*) cell->mem;
            pp.size = entry_size();
            return pp;
        }
        assert (diff < 0);
        return pp;
    }
    //--------------------------------------------------------------------------
    void bounded_push_commit (const mem_mpmc_b_prepared& pp)
    {
        if (pp.cell) {
            ((cell_t*) pp.cell)->sequence.store (pp.pos, mo_release);
        }
    }
    //--------------------------------------------------------------------------
    mem_mpmc_b_prepared mc_pop_prepare()
    {
        assert (m_buffer);
        mem_mpmc_b_prepared pp;
        cell_t* cell;
        size_t pos = m_dequeue_pos;
        while (true) {
            cell          = &m_buffer[pos & m_buffer_mask];
            size_t seq    = cell->sequence.load (mo_acquire);
            intptr_t diff = (intptr_t) seq - (intptr_t) (pos + 1);
            if (diff == 0) {
                if (m_dequeue_pos.compare_exchange_weak(
                    pos, pos + 1, mo_relaxed
                    )) {
                    break;
                }
            }
            else if (diff < 0) {
                return pp;
            }
            else {
                pos = m_dequeue_pos;
            }
        }
        pp.cell = cell;
        pp.pos  = pos + m_buffer_mask + 1;
        pp.mem  = (u8*) cell->mem;
        pp.size = entry_size();
        return pp;
    }
    //--------------------------------------------------------------------------
    mem_mpmc_b_prepared sc_pop_prepare()
    {
        assert (m_buffer);
        mem_mpmc_b_prepared pp;
        cell_t* cell  = &m_buffer[m_dequeue_pos & m_buffer_mask];
        size_t seq    = cell->sequence.load (mo_acquire);
        intptr_t diff = (intptr_t) seq - (intptr_t) (m_dequeue_pos + 1);
        if (diff == 0) {
            ++m_dequeue_pos;
            pp.cell = cell;
            pp.pos  = m_dequeue_pos + m_buffer_mask;
            pp.mem  = (u8*) cell->mem;
            pp.size = entry_size();
            return pp;
        }
        assert (diff < 0);
        return pp;
    }
    //--------------------------------------------------------------------------
    void pop_commit (const mem_mpmc_b_prepared& pp)
    {
        if (pp.cell) {
            ((cell_t*) pp.cell)->sequence.store (pp.pos, mo_release);
        }
    }
    //--------------------------------------------------------------------------
private:
    //--------------------------------------------------------------------------
    struct cell_t
    {
        at::atomic<size_t> sequence;
        void*              mem;
    };
    //--------------------------------------------------------------------------
    typedef char cacheline_pad_t [cache_line_size];

    cacheline_pad_t           m_pad0;

    cell_t*                   m_buffer;
    size_t                    m_buffer_mask;
    size_t                    m_entry_size;
    u8*                       m_mem;

    cacheline_pad_t           m_pad1;

    mo_relaxed_atomic<size_t> m_enqueue_pos;

    cacheline_pad_t           m_pad2;

    mo_relaxed_atomic<size_t> m_dequeue_pos;

    cacheline_pad_t           m_pad3;

    mem_mpmc_b (mem_mpmc_b const&);
    void operator= (mem_mpmc_b const&);
};
//------------------------------------------------------------------------------
} //namespaces

#endif /* MAL_LOG_MEMORY_MPMC_BOUNDED_HPP_ */

#endif
