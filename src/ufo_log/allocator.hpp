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

#ifndef UFO_LOG_LOG_ALLOCATOR_HPP_
#define UFO_LOG_LOG_ALLOCATOR_HPP_

#include <new>
#include <cassert>
#include <type_traits>
#include <vector>
#include <algorithm>
#include <ufo_log/util/mpsc.hpp>
#include <ufo_log/util/mpmc_bounded.hpp>
#include <ufo_log/util/integer_bits.hpp>
#include <ufo_log/util/on_stack_dynamic.hpp>

namespace ufo {

//This is a very specific spmc allocator for just this scenario and shouldn't
//be used as a generic one.

//The main reason to develop it isn't performance, as a matter of fact it barely
//beats the heap. The main reason to have it is to be able to tune the logger
//memory usage when the heap is disabled.

//An atomic byte counter with fetch add and fetch sub would do the same job with
//less complexity, but having multiple threads modifying the same variable is
//a no-go, so this is the simplest next step, the good old block allocator.

//Considering that the size is known and just a single thread will deallocate
//this has a lot of room for improvement.

//The only thread safe functions are allocate and deallocate.

//The functions intended to be used by the producer never throw.
//------------------------------------------------------------------------------
class ufo_allocator
{
public:
    //--------------------------------------------------------------------------
    ufo_allocator()
    {
        zero();
    };
    //--------------------------------------------------------------------------
    ~ufo_allocator()
    {
        free();
    };
    //--------------------------------------------------------------------------
    bool init (uword total_byte_size, uword entry_size, bool use_heap)
    {
        assert ((total_byte_size && entry_size) || use_heap);
        assert (entry_size <= total_byte_size);
        assert (!m_fixed_begin && !m_use_heap);

        uword entries = 0;
        if (total_byte_size && entry_size)
        {
            total_byte_size = next_pow2 (total_byte_size);
            entry_size      = next_pow2 (entry_size);

            if ((entry_size <= total_byte_size))
            {
                entries = total_byte_size / entry_size;
            }
        }
        if (!entries)
        {
            zero();
            m_use_heap = use_heap;
            return use_heap;
        }
        m_fixed_begin = (u8*) ::operator new (total_byte_size, std::nothrow);
        if (!m_fixed_begin)
        {
            return false;
        }
        try
        {
            m_free.construct (entries);
        }
        catch (...)
        {
            free();
            return false;
        }
        m_fixed_end     = m_fixed_begin + total_byte_size;
        m_entry_size    = entry_size;
        m_use_heap      = use_heap;

#ifdef EXPERIMENTAL_ORDERING_CACHE
        uword reorder_count;
        reorder_count   = fixed_size_entries / 25;
        reorder_count   = (reorder_count == 0) ? 1 : 0;
        reorder_count   = (reorder_count > reorder_buffer_size) ?
                                reorder_buffer_size : reorder_count;
        m_reorder_count = reorder_count;
        m_reorder.reserve (m_reorder_count);
#endif
        for (uword i = 0; i < entries; ++i)
        {
            m_free->sp_bounded_push (m_fixed_begin + (i * m_entry_size));
        }
        return true;
    }
    //--------------------------------------------------------------------------
    void free()
    {
        if (m_fixed_begin)
        {
            m_free.destruct_if();
#ifdef EXPERIMENTAL_ORDERING_CACHE
            m_reorder.clear();
#endif
            ::operator delete (m_fixed_begin);
            zero();
        }
    }
    //--------------------------------------------------------------------------
    void* allocate (uword size)
    {
        if (size)
        {
            void* ret;
            if ((size <= m_entry_size) && m_free->mc_pop (ret))                 //TODO: profile if this scenario beats the heap
            {
                return ret;
            }
            else if (m_use_heap)
            {
                return operator new (size);
            }
        }
        return nullptr;
    }
    //--------------------------------------------------------------------------
    bool deallocate (void* p, uword size = 0)                                   //0 = unused. All the cases returning false are bugs
    {
        uword addr = (uword) p;
        uword fbeg = (uword) m_fixed_begin;
        uword fend = (uword) m_fixed_end;

        if ((addr >= fbeg) && (addr < fend))
        {
            addr -= fbeg;
            if ((addr & (m_entry_size - 1)) == 0)
            {
#ifndef EXPERIMENTAL_ORDERING_CACHE
                m_free->sp_bounded_push (p);
#else
                fixed_deallocate (p);
#endif
                return true;
            }
            assert (false && "returned pointer misaligned");
            return false;
        }
        else if (m_use_heap)
        {
            try
            {
                operator delete (p);
            }
            catch (...)
            {
                assert (false && "memory corruption in delete");
                return false;
            }
            return true;
        }
        else
        {
            assert (false && "memory corruption");
            return false;
        }
    }
    //--------------------------------------------------------------------------
private:
#ifdef EXPERIMENTAL_ORDERING_CACHE
    //--------------------------------------------------------------------------
    void fixed_deallocate (void* p)
    {
        m_reorder.push_back ((uword) p);
        if (m_reorder.size() == m_reorder_count.load (mo_relaxed))
        {
            return_nodes();
        }
    }
    //--------------------------------------------------------------------------
    void return_nodes()
    {
        std::sort (m_reorder.begin(), m_reorder.end());                         //naive in-place is fine for the used sizes
        for (uword i = 0; i < m_reorder.size(); ++i)
        {
            m_free->sp_bounded_push ((void*) m_reorder[i]);
        }
        m_reorder.clear();
    }
    //--------------------------------------------------------------------------
    static const uword reorder_buffer_size = 32;
    static const uword reorder_divisor     = 10;
#endif
    //--------------------------------------------------------------------------
    void zero()
    {
        m_entry_size    = 0;
        m_fixed_begin   = m_fixed_end = nullptr;
        m_use_heap      = false;
#ifdef EXPERIMENTAL_ORDERING_CACHE
        m_reorder_count = 1;
#endif
    }
    //--------------------------------------------------------------------------
    ufo_allocator (const ufo_allocator& other);
    ufo_allocator& operator= (const ufo_allocator& other);

    typedef mpmc_b_fifo<void*> free_list;
    //--------------------------------------------------------------------------
    placement_new<void*, cache_line_size> m_pad1;
    uword                                 m_entry_size;
    uint8*                                m_fixed_begin;
    uint8*                                m_fixed_end;
    bool                                  m_use_heap;
    on_stack_dynamic<free_list>           m_free;
#ifdef EXPERIMENTAL_ORDERING_CACHE
    at::atomic<uword>                     m_reorder_count;
    std::vector<uword>                    m_reorder;
#endif
    //--------------------------------------------------------------------------
}; //class allocator
//------------------------------------------------------------------------------
} //namespaces

#endif /* UFO_LOG_LOG_ALLOCATOR_HPP_ */
