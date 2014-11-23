/*
 * raw_circular_buffer.hpp
 *
 *  Created on: Nov 23, 2014
 *      Author: rafa
 */

#ifndef UFO_LOG_RAW_CIRCULAR_BUFFER_HPP_
#define UFO_LOG_RAW_CIRCULAR_BUFFER_HPP_

#include <cassert>
#include <new>
#include <ufo_log/util/integer.hpp>

namespace ufo {
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
        if (elem_size && elem_count)
        {
            m_entry_size = elem_size;
            m_total_size = elem_size * elem_count;
            if ((m_total_size / m_entry_size) == elem_count)
            {
                return allocate();
            }
        }
        return false;
    }
    //--------------------------------------------------------------------------
    void free()
    {
        if (m_mem)
        {
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
        if (m_mem)
        {
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

#endif /* UFO_LOG_RAW_CIRCULAR_BUFFER_HPP_ */
