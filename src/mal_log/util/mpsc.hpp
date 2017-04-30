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

#ifndef MAL_MPSC_HPP_
#define MAL_MPSC_HPP_

#include <type_traits>
#include <mal_log/util/system.hpp>
#include <mal_log/util/aligned_type.hpp>
#include <mal_log/util/atomic.hpp>

namespace mal {

//------------------------------------------------------------------------------
struct mpsc_node_hook
{
    typedef mo_relaxed_atomic<mpsc_node_hook*> ptr_type;
    ptr_type next;
};
//------------------------------------------------------------------------------
struct mpsc_result
{
    enum error_type {
        no_error,
        empty,
        busy_try_again
    };

    explicit mpsc_result(
            error_type error = empty, mpsc_node_hook* node = nullptr) :
            error (error), node (node)
    {};
    error_type      error;
    mpsc_node_hook* node;
};

//------------------------------------------------------------------------------
class mpsc_i_fifo
{
public:
    //--------------------------------------------------------------------------
    mpsc_i_fifo() : m_tail(), m_head(), m_stable_node()
    {
        reset();
    }
    //--------------------------------------------------------------------------
    ~mpsc_i_fifo() {};
    //--------------------------------------------------------------------------
    void reset()                                                                 //NOT THREAD SAFE! very dangerous function to call! not meant to be called during use, just in very safe conditions when it is known that there is no danger, e.g. when you know that you have deallocated every item contained to the queue and you want to reuse the instance.
    {
        *m_head = &m_stable_node.get();
        *m_tail = &m_stable_node.get();
        m_stable_node->next = nullptr;
    }
    //--------------------------------------------------------------------------
    bool push (mpsc_node_hook& n)
    {
        n.next = nullptr;
        mpsc_node_hook* old_tail = m_tail->exchange (&n, mo_acq_rel);
        //(*)                                                                   another push() here would succeed, a pop() when the queue size is > 1 would succeed too.
        old_tail->next.store (&n, mo_release);
        return old_tail == &m_stable_node.get();                                      //returns if the queue was empty in most cases, it can give false positives when having high contention, you can use it for e.g. to awake hybrid locks if you can tolerate some false positives
    }
    //--------------------------------------------------------------------------
    bool push_many (mpsc_node_hook& first, mpsc_node_hook& last)
    {
        last.next = nullptr;
        mpsc_node_hook* old_tail = m_tail->exchange (&last, mo_acq_rel);
        //(*)                                                                   another push() here would succeed, a pop() when the queue size is > 1 would succeed too.
        old_tail->next.store (&first, mo_release);
        return old_tail == &m_stable_node.get();                                      //returns if the queue was empty in most cases, it can give false positives when having high contention, you can use it for e.g. to awake hybrid locks if you can tolerate some false positives
    }
    //--------------------------------------------------------------------------
    mpsc_result pop()
    {
        mpsc_node_hook* first_node  = *m_head;
        mpsc_node_hook* second_node = first_node->next.load (mo_acquire);
        if (first_node == &m_stable_node.get()) {
            if (second_node)  {                                                       // non empty, but as the stable node has no useful data (is used to save a a compare exchange strong) we need to update the head and local vars before proceeding.
                *m_head     = second_node;
                first_node  = second_node;
                second_node = second_node->next;
            }
            else {
                return mpsc_result (mpsc_result::empty);
            }
        }
        if (second_node) {                                                            // (completely commited pushes > 1), we don't need to care about what happens in the tail (arbitrate with the stable node).
            *m_head = second_node;
            return mpsc_result (mpsc_result::no_error, first_node);
        }
                                                                                // (completely commited pushes <= 1 from here and below) first_node.next = second_node = nullptr
        mpsc_node_hook* tail = *m_tail;
        if (first_node != tail) {                                               // if the first node isn't the tail the memory snapshot that we have shows that someone is pushing just now. The snapshot comes from the moment in time marked with an asterisk in a comment line in the push() function. We have unconsistent data and we need to wait.
            return mpsc_result (mpsc_result::busy_try_again);
        }
        push (*m_stable_node);                                                  // insert the stable node, so we can detect contention with other pushes.
        second_node = first_node->next;                                         // remember that "push" had a full fence, the view now is consistent.
        if (second_node) {                                                      // "first_node.next" should point to either the stable node or something new that was completely pushed in between.
            *m_head = second_node;
            return mpsc_result (mpsc_result::no_error, first_node);
        }
        else {                                                                  // we see the nullptr in next from a push that came before ours and the stable node is inserted after this new node. this new push that came is incomplete (at the asterisk (*) point)
            return mpsc_result (mpsc_result::busy_try_again);
        }
    }
    //--------------------------------------------------------------------------
private:
    //--------------------------------------------------------------------------
    aligned_type<void*, cache_line_size>                    m_pad1;
    aligned_type<mpsc_node_hook::ptr_type, cache_line_size> m_tail;
    aligned_type<mpsc_node_hook::ptr_type, cache_line_size> m_head;
    aligned_type<mpsc_node_hook, cache_line_size>           m_stable_node;
    aligned_type<void*, cache_line_size>                    m_pad2;
    //--------------------------------------------------------------------------
};

//------------------------------------------------------------------------------
class mpsc_ie_fifo //like the one above, but this one returns if the queue was empty before pushing without false positives
{
public:
    //--------------------------------------------------------------------------
    mpsc_ie_fifo() : m_tail(), m_head(), m_stable_node()
    {
        reset();
    }
    //--------------------------------------------------------------------------
    ~mpsc_ie_fifo() {};
    //--------------------------------------------------------------------------
    void reset()                                                                //NOT THREAD SAFE! very dangerous function to call! not meant to be called during use, just in very safe conditions when it is known that there is no danger, e.g. when you know that you have deallocated every item contained to the queue and you want to reuse the instance.
    {
        *m_head = &m_stable_node.get();
        *m_tail = tag (&m_stable_node.get());
        m_stable_node->next = nullptr;
    }
    //--------------------------------------------------------------------------
    bool push (mpsc_node_hook& n)
    {
        n.next = nullptr;
        mpsc_node_hook* old_tail = m_tail->exchange (&n, mo_acq_rel);
        //(*)                                                                   another push() here would succeed, a pop() when the queue size is > 1 would succeed too.
        bool was_empty           = is_tagged (old_tail);
        untag (old_tail)->next.store (&n, mo_release);
        return was_empty;
    }
    //--------------------------------------------------------------------------
    bool push_many (mpsc_node_hook& first, mpsc_node_hook& last)
    {
        last.next = nullptr;
        mpsc_node_hook* old_tail = m_tail->exchange (&last, mo_acq_rel);
        //(*)                                                                   another push() here would succeed, a pop() when the queue size is > 1 would succeed too.
        bool was_empty           = is_tagged (old_tail);
        untag (old_tail)->next.store (&first, mo_release);
        return was_empty;
    }
    //--------------------------------------------------------------------------
    mpsc_result pop()
    {
        mpsc_node_hook* first_node  = *m_head;
        mpsc_node_hook* second_node = first_node->next.load (mo_acquire);
        if (first_node == &m_stable_node.get()) {
            if (second_node) {                                                   // non empty, but as the stable node has no useful data (is used to save a a compare exchange strong) we need to update the head and local vars before proceeding.
                *m_head     = second_node;
                first_node  = second_node;
                second_node = second_node->next;
            }
            else {
                return mpsc_result (mpsc_result::empty);
            }
        }
        if (second_node) {                                                      // (completely commited pushes > 1), we don't need to care about what happens in the tail (arbitrate with the stable node).
            *m_head = second_node;
            return mpsc_result (mpsc_result::no_error, first_node);
        }                                                                       // (completely commited pushes <= 1 from here and below) first_node.next = second_node = nullptr
        mpsc_node_hook* tail = *m_tail;
        if (first_node != tail) {                                               // if the first node isn't the tail the memory snapshot that we have shows that someone is pushing just now. The snapshot comes from the moment in time marked with an asterisk in a comment line in the push() function. We have unconsistent data and we need to wait.
            return mpsc_result (mpsc_result::busy_try_again);
        }
        else {
            m_stable_node->next = nullptr;
            if (m_tail->compare_exchange_strong(
                    first_node, tag (&m_stable_node.get()), mo_release
                    )) {
                *m_head = &m_stable_node.get();
                return mpsc_result (mpsc_result::no_error, first_node);
            }
            return mpsc_result (mpsc_result::busy_try_again);                   // if the first node isn't the tail the memory snapshot that we have shows that someone is pushing just now. The snapshot comes from the moment in time marked with an asterisk in a comment line in the push() function. We have unconsistent data and we need to wait.
        }
    }
    //--------------------------------------------------------------------------
private:
    //--------------------------------------------------------------------------
    inline bool is_tagged (mpsc_node_hook* const p)
    {
        return (((uword) p) & 1) ? true : false;
    }
    //--------------------------------------------------------------------------
    inline mpsc_node_hook* tag (mpsc_node_hook* p)
    {
        return (mpsc_node_hook*) ((uword) p | 1);
    }
    //--------------------------------------------------------------------------
    inline mpsc_node_hook* untag (mpsc_node_hook* p)
    {
        return (mpsc_node_hook*) ((uword) p & ~1);
    }
    //--------------------------------------------------------------------------
    aligned_type<void*, cache_line_size>                    m_pad1;
    aligned_type<mpsc_node_hook::ptr_type, cache_line_size> m_tail;
    aligned_type<mpsc_node_hook::ptr_type, cache_line_size> m_head;
    aligned_type<mpsc_node_hook, cache_line_size>           m_stable_node;
    aligned_type<void*, cache_line_size>                    m_pad2;
};
//------------------------------------------------------------------------------

} //namespace

#endif /* MAL_MPSC_HPP_ */
