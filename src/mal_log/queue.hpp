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

#ifndef MAL_LOG_QUEUE_HPP_
#define MAL_LOG_QUEUE_HPP_

#include <cassert>
#include <new>
#include <stddef.h>
#include <mal_log/util/mpsc.hpp>
#include <mal_log/util/integer_bits.hpp>
#include <mal_log/util/atomic.hpp>
#include <mal_log/util/thread.hpp>
#include <mal_log/util/chrono.hpp>

namespace mal {

//------------------------------------------------------------------------------
class queue_prepared
{
public:
    enum error {
        success,
        queue_full,
        entry_size,
        queue_blocked,
        count,
    };
    queue_prepared()
    {
        mem = nullptr;
        pos = 0;
    }
    u8* get_mem() const
    {
        return ((uword) mem >= count) ? mem : nullptr;
    }
    error get_error() const
    {
        return (((uword) mem) < count) ? *((error*) &mem) : success;
    }
    size_t transaction_value()
    {
        return pos;
    }
private:
    friend class queue;
    void set_error (error e, size_t pos)
    {
        *((error*) &mem) = e;
    }
    void set (u8* mem, size_t pos)
    {
        this->mem = mem;
        this->pos = pos;
    }
    u8*    mem;
    size_t pos;
};
//------------------------------------------------------------------------------
// Roughly explained this is the Dmytry MPMC queue converted to MPSC, and broken
// into prepare/commit blocks that allow the use of a parallel heap based queue
// to be optionally used when the main fixed sized queue can't allocate entries
// (either because the queue is full or because a given entry exceeds the fixed
// entry size).
//
// The queue is still linearizable.
//------------------------------------------------------------------------------
class queue
{
private:
    //--------------------------------------------------------------------------
    struct heap_node : public mpsc_node_hook
    {
        //----------------------------------------------------------------------
        u8* storage()
        {
            return ((u8*) this) + sizeof *this;
        }
        //----------------------------------------------------------------------
        static heap_node* from_storage (u8* mem)
        {
            return (heap_node*) (mem - sizeof (heap_node));
        }
        //----------------------------------------------------------------------
        static uword strict_total_size (uword effective_size)
        {
            return sizeof (heap_node) + effective_size;
        }
        //----------------------------------------------------------------------
        static uword effective_size (uword total_size)
        {
            auto v = total_size - sizeof (heap_node);
            assert (v < total_size);
            return v;
        }
        //----------------------------------------------------------------------
        size_t pos;
    };
    //--------------------------------------------------------------------------
    struct local_cell
    {
        at::atomic<size_t> sequence;
        //----------------------------------------------------------------------
        u8* storage()
        {
            return ((u8*) this) + sizeof *this;
        }
        //----------------------------------------------------------------------
        static local_cell* from_storage (u8* p)
        {
            return (local_cell*) (p - sizeof (local_cell));
        }
        //----------------------------------------------------------------------
        static uword strict_total_size (uword effective_size)
        {
            return sizeof (local_cell) + effective_size;
        }
        //----------------------------------------------------------------------
        static uword effective_size (uword total_size)
        {
            auto v = total_size - sizeof (local_cell);
            assert (v < total_size);
            return v;
        }
        //----------------------------------------------------------------------
    };
    //--------------------------------------------------------------------------
    enum mode {
        blocked,
        bounded,
        hybrid,
        heap,
    };

    bool mode_allows_heap() const
    {
        return m_mode >= hybrid;
    }

public:
    //--------------------------------------------------------------------------
    queue()
    {
        m_bounded_mem = nullptr;
        clear();
    }
    //--------------------------------------------------------------------------
    ~queue()
    {
        mpsc_result r;
        do {
            r = m_heap_fifo.pop();
            if (r.error == mpsc_result::no_error) {
                assert (false && "user didn't cleanup");
                auto n = (heap_node*) r.node;
                ::operator delete (n, std::nothrow);
            }
        }
        while (r.error != mpsc_result::empty);
        if (m_heap_pop.mem) {
            assert (false && "user didn't cleanup");
            heap_node* n = heap_node::from_storage (m_heap_pop.mem);
            ::operator delete (n, std::nothrow);
        }
        clear();
    }
    //--------------------------------------------------------------------------
    void clear()                                                                //Dangerous, just to be used after failed initializations
    {
        if (m_bounded_mem) {
            ::operator delete (m_bounded_mem);
        }
        m_bounded_mem     = nullptr;
        m_bounded_mem_end = nullptr;
        m_mode            = bounded;
        m_cell_mask       = 0;
        m_entry_size      = 0;
        m_enqueue_pos     = 0;
        m_dequeue_pos     = 0;
    }
    //--------------------------------------------------------------------------
    static const size_t min_entries = 256; // bigger (hopefully) than the thread count
    static const size_t min_entry_bytes = 32;
    //--------------------------------------------------------------------------
    static bool validate_bounded_q_size_constraints(
        size_t fixed_bytes, size_t fixed_entries
        )
    {
        return fixed_entries >= min_entries
            && (fixed_entries & (fixed_entries - 1)) == 0
            && fixed_bytes >= fixed_entries
            && fixed_entries <= (((size_t) -1) >> 2); //wrap bit + qblock bit
    }
    //--------------------------------------------------------------------------
    static bool validate_size_constraints(
        size_t fixed_bytes, size_t fixed_entries, bool can_use_heap
        )
    {
        bool bounded = validate_bounded_q_size_constraints(
            fixed_bytes, fixed_entries
            );
        bool unbounded =
            can_use_heap
            && (fixed_bytes == 0)
            && (fixed_entries == 0);
        return bounded || unbounded;
    }
    //--------------------------------------------------------------------------
    bool init (size_t fixed_bytes, size_t fixed_entries, bool can_use_heap)
    {
        static const uword align = std::alignment_of<local_cell>::value;

        if (initialized()) { return false; }

        if (validate_bounded_q_size_constraints (fixed_bytes, fixed_entries)) {
            clear();
            fixed_bytes   = (fixed_bytes / fixed_entries) * fixed_entries;   //just rounding...
            m_entry_size  = fixed_bytes / fixed_entries;
            m_entry_size  = local_cell::strict_total_size (m_entry_size);
            m_entry_size  = align * div_ceil (m_entry_size, align);
            m_enqueue_pos = 0;
            m_dequeue_pos = 0;
            m_cell_mask   = fixed_entries - 1;
            m_bounded_mem = (u8*) ::operator new(
                m_entry_size * fixed_entries, std::nothrow
                );
            if (!m_bounded_mem) {
                return false;
            }
            m_bounded_mem_end = m_bounded_mem + (m_entry_size * fixed_entries);
            for (size_t i = 0; i < fixed_entries; ++i) {
                get_cell (i)->sequence = i;
            }
            m_mode = can_use_heap ? hybrid : bounded;
            return true;
        }
        else if (can_use_heap && (fixed_bytes == 0) && (fixed_entries == 0)) {
            clear();
            m_mode = heap;
            return true;
        }
        return false;
    }
    //--------------------------------------------------------------------------
    bool initialized() const
    {
        return m_bounded_mem || mode_allows_heap();
    }
    //--------------------------------------------------------------------------
    size_t fixed_entry_size() const
    {
        return local_cell::effective_size (m_entry_size);
    }
    //--------------------------------------------------------------------------
    void block_producers() /*this is for termination contexts*/
    {
        m_enqueue_pos.fetch_add (queue_blocked_offset(), mo_relaxed);
        /*ugly: there is no way to block the MPSC intrusive queue, a grace
          period needs to be added to ensure memory visibility from all cores.
          As this is to be used only in termination contexts it isn't a problem.
          A right implementation would require to screw up the queue
          throughput. 100ms is a brutal overshoot.*/
        m_mode = blocked;
        th::this_thread::sleep_for (ch::milliseconds (100));
    }
    //--------------------------------------------------------------------------
    queue_prepared mp_bounded_push_prepare (size_t size)
    {
        queue_prepared pp;
        if (m_bounded_mem && (size <= fixed_entry_size())) {
            local_cell* cell;
            size_t pos = m_enqueue_pos.load (mo_relaxed);
            while (true) {
                cell          = get_cell (pos & m_cell_mask);
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
                    if (mode_allows_heap()) {
                        alloc_from_heap (pp, size, pos);
                    }
                    else if (diff > -queue_blocked_offset()) {
                        /*this error sets pos to the value that a consumer would
                          find on its returned "sc_pop_prepare" pos. This can be
                          used e.g. to know if a producer is blocked before this
                          transaction.*/
                        pp.set_error(
                            queue_prepared::queue_full, pos - entry_count()
                            );
                    }
                    else {
                        pp.set_error (queue_prepared::queue_blocked, 0);
                    }
                    return pp;
                }
                else {
                    pos = m_enqueue_pos;
                }
            }
            pp.set (cell->storage(), pos + 1);
        }
        else if (mode_allows_heap()) {
            alloc_from_heap(
                pp, size, m_bounded_mem ? m_enqueue_pos.load (mo_relaxed) : 0
                );
        }
        else {
            //no alloc, size > fixed_entry_size()
            pp.set_error(
                m_mode != blocked ?
                    queue_prepared::entry_size :
                    queue_prepared::queue_blocked,
                0
                );
        }
        return pp;
    }
    //--------------------------------------------------------------------------
    queue_prepared mp_bounded_push_next_entry_reserve()
    {
        queue_prepared pp;
        size_t pos = m_enqueue_pos.fetch_add (1, mo_relaxed);
        pp.set (get_cell (pos & m_cell_mask)->storage(), pos + 1);
        return pp;
    }
    //--------------------------------------------------------------------------
    queue_prepared::error mp_bounded_push_reserved_entry_is_ready(
        const queue_prepared& pp
        )
    {
        auto cell  = local_cell::from_storage (pp.mem);
        size_t pos = pp.pos - 1;
        size_t seq = cell->sequence.load (mo_acquire);
        intptr_t diff = (intptr_t) seq - (intptr_t) pos;
        assert (diff <= 0);
        if (diff == 0) {
            return queue_prepared::success;
        }
        size_t posnow = m_enqueue_pos.load (mo_relaxed);
        auto diffnow  = (intptr_t) posnow - (intptr_t) pos;
        return diffnow < entry_count() ?
            queue_prepared::queue_full : queue_prepared::queue_blocked;
    }
    //--------------------------------------------------------------------------
    void bounded_push_commit (const queue_prepared& pp)
    {
        assert (pp.get_mem());
        if (is_local_mem (pp.mem)) {
            auto cell = local_cell::from_storage (pp.mem);
            cell->sequence.store (pp.pos, mo_release);
        }
        else {
            heap_node* n = heap_node::from_storage (pp.mem);
            m_heap_fifo.push (*n);
        }
    }
    //--------------------------------------------------------------------------
    queue_prepared sc_pop_prepare()
    {
        queue_prepared pp;
        bool again = false;
    try_again:
        if ((mode_allows_heap() || m_mode == blocked) &&
            m_heap_pop.mem == nullptr
            ) {
            auto res = m_heap_fifo.pop();
            if (res.error == mpsc_result::no_error) {
                heap_node* n   = (heap_node*) res.node;
                m_heap_pop.set (n->storage(), n->pos);
                again = false;
            }
            else if (res.error == mpsc_result::busy_try_again) {
                again = true;
            }
        }
        if (m_bounded_mem && (m_fixed_pop.mem == nullptr)) {
            local_cell* cell = get_cell (m_dequeue_pos & m_cell_mask);
            size_t seq       = cell->sequence.load (mo_acquire);
            auto pos         = m_dequeue_pos.load (mo_relaxed);
            intptr_t diff    = (intptr_t) seq - (intptr_t) (pos + 1);
            if (diff == 0) {
                m_fixed_pop.set (cell->storage(), pos);
                m_dequeue_pos = pos + 1;
            }
        }
        if (again) {
            goto try_again;
        }
        if (m_heap_pop.mem && m_fixed_pop.mem) {
            if (m_fixed_pop.pos < m_heap_pop.pos) {
                pp              = m_fixed_pop;
                m_fixed_pop.mem = nullptr;
            }
            else {
                pp             = m_heap_pop;
                m_heap_pop.mem = nullptr;
            }
        }
        else if (m_heap_pop.mem) {
            pp             = m_heap_pop;
            m_heap_pop.mem = nullptr;
        }
        else if (m_fixed_pop.mem) {
            pp              = m_fixed_pop;
            m_fixed_pop.mem = nullptr;
        }
        return pp;
    }
    //--------------------------------------------------------------------------
    void pop_commit (const queue_prepared& pp)
    {
        assert (pp.get_mem());
        if (is_local_mem (pp.mem)) {
            auto cell = local_cell::from_storage (pp.mem);
            cell->sequence.store (pp.pos + entry_count(), mo_release);
        }
        else {
            heap_node* n = heap_node::from_storage (pp.mem);
            ::operator delete (n, std::nothrow);
        }
    }
    //--------------------------------------------------------------------------
    size_t entry_count()
    {
        return m_cell_mask + 1;
    }
    //--------------------------------------------------------------------------
private:
    //--------------------------------------------------------------------------
    size_t queue_blocked_offset()
    {
        return 2 * (entry_count());
    }
    //--------------------------------------------------------------------------
    inline bool is_local_mem (const u8* mem) const
    {
        auto beg  = (uword) m_bounded_mem;
        auto end  = (uword) m_bounded_mem_end;
        auto addr = (uword) mem;
        return (addr >= beg) && (addr < end);
    }
    //--------------------------------------------------------------------------
    void alloc_from_heap (queue_prepared& pp, size_t sz, size_t pos)
    {
        if (auto n = (heap_node*) operator new(
            heap_node::strict_total_size (sz), std::nothrow
            )) {
            pp.set (n->storage(), pos);
            n->pos = pos;
        }
    }
    //--------------------------------------------------------------------------
    local_cell* get_cell (uword i)
    {
        assert (m_bounded_mem);
        local_cell* ret = (local_cell*) (m_bounded_mem + (i * m_entry_size));
        assert ((uword) ret < (uword) m_bounded_mem_end);
        return ret;
    }
    //--------------------------------------------------------------------------
    typedef char cacheline_pad_t [cache_line_size];

    cacheline_pad_t           m_pad0;

    size_t                    m_cell_mask;
    size_t                    m_entry_size;
    u8*                       m_bounded_mem;
    u8*                       m_bounded_mem_end;
    mode                      m_mode;

    cacheline_pad_t           m_pad1;

    mo_relaxed_atomic<size_t> m_enqueue_pos;

    cacheline_pad_t           m_pad2;

    mo_relaxed_atomic<size_t> m_dequeue_pos;
    queue_prepared            m_heap_pop, m_fixed_pop;

    mpsc_i_fifo               m_heap_fifo;                                      //some extra memory locality could be gained by wrapping both queues.

    queue (queue const&);
    void operator= (queue const&);
};
//------------------------------------------------------------------------------
} //namespaces

#endif /* MAL_LOG_QUEUE_HPP_ */
