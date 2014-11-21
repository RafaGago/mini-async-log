/*
 * log_allocator.hpp
 *
 *  Created on: Nov 14, 2014
 *      Author: rafgag
 */


#ifndef TINY_LOG_LOG_ALLOCATOR_HPP_
#define TINY_LOG_LOG_ALLOCATOR_HPP_

#include <new>
#include <cassert>
#include <type_traits>
#include <tiny/util/mpsc.hpp>
#include <tiny/util/spmc.hpp>
#include <tiny/util/integer_bits.hpp>

namespace tiny {

//this is a very specific spmc allocator, just the log backend thread can
//deallocate!

//The only thread safe functions are allocate and deallocate.

//The functions intended to be used by the producer never throw.

//My point is, this is a class and looks like it can be reused, but the fact
//is that it's pretty specialized and just separated to a header to avoid
//cluttering the log backend implementation.

//------------------------------------------------------------------------------
class tiny_allocator
{
public:
    //--------------------------------------------------------------------------
    tiny_allocator()
    {
        zero();
    };
    //--------------------------------------------------------------------------
    ~tiny_allocator()
    {
        if (m_fixed_begin)
        {
            ::operator delete (m_fixed_begin);
            zero();
        }
    };
    //--------------------------------------------------------------------------
    bool init(
        uint16 fixed_size_entries,
        uint16 fixed_size_entry_size,
        bool   use_heap
        )
    {
        assert ((fixed_size_entries && fixed_size_entry_size) || use_heap);
        assert (!m_fixed_begin && !m_use_heap);

        uword entries  = 0;
        uword entry_sz = 0;
        uword bsz      = 0;

        if (fixed_size_entries && fixed_size_entry_size)
        {
            entries  = next_pow2 ((uword) fixed_size_entries);
            entry_sz = next_pow2 ((uword) fixed_size_entry_size);
            bsz      = entries * entry_sz;
        }

        if (!bsz)
        {
            zero();
            m_use_heap = use_heap;
            return true;
        }

        m_fixed_begin  = (u8*) ::operator new (bsz, std::nothrow);
        if (!m_fixed_begin)
        {
            return false;
        }
        try
        {
            new (&m_freelist_storage) free_list (entries);
        }
        catch (...)
        {
            ::operator delete (m_fixed_begin);
            zero();
            return false;
        }
        m_fixed_end   = m_fixed_begin + bsz;
        m_entry_size  = entry_sz;
        m_use_heap    = use_heap;
        for (uword i = 0; i < entries; ++i)
        {
            get_free_list().bounded_push (m_fixed_begin + (i * m_entry_size));
        }
        return true;
    }
    //--------------------------------------------------------------------------
    void* allocate (uword size)
    {
        if (size)
        {
            void* ret;
            if ((size <= m_entry_size) && get_free_list().pop (ret))            //TODO: profile if this scenario beats the heap
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
                get_free_list().bounded_push (p);
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

    //--------------------------------------------------------------------------
    tiny_allocator (const tiny_allocator& other);
    tiny_allocator& operator= (const tiny_allocator& other);

    typedef spmc_b_fifo<void*>                   free_list;
    typedef std::aligned_storage<
            sizeof (free_list),
            std::alignment_of<free_list>::value
        >::type                                  free_list_storage;
    //--------------------------------------------------------------------------
    void zero()
    {
        m_entry_size  = 0;
        m_fixed_begin = m_fixed_end = nullptr;
        m_use_heap    = false;
    }
    //--------------------------------------------------------------------------
    free_list& get_free_list()
    {
        return *((free_list*) &m_freelist_storage);
    }
    //--------------------------------------------------------------------------
    uword             m_entry_size;
    uint8*            m_fixed_begin;
    uint8*            m_fixed_end;
    free_list_storage m_freelist_storage;
    bool              m_use_heap;
    //--------------------------------------------------------------------------
}; //class allocator
//------------------------------------------------------------------------------
} //namespaces

#endif /* TINY_LOG_LOG_ALLOCATOR_HPP_ */
