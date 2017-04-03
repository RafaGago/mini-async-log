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

#ifndef MAL_BACKOFF_WAIT_HPP_
#define MAL_BACKOFF_WAIT_HPP_

#include <mal_log/util/thread.hpp>
#include <mal_log/util/atomic.hpp>
#include <mal_log/util/chrono.hpp>
#include <mal_log/util/mpsc.hpp>
#include <mal_log/queue.hpp>
#include <mal_log/util/processor_pause.hpp>
#include <stdlib.h>

namespace mal {

class backoff_wait;
//------------------------------------------------------------------------------
class backoff_ticket : public mpsc_node_hook {
public:
    //--------------------------------------------------------------------------
    backoff_ticket()
    {
        m_state.store (unitialized, mo_relaxed);
    }
    //--------------------------------------------------------------------------
    queue_prepared* get_last_q_element()
    {
        return (get_state() == sent_q_last) ? &m_last_q_elem : nullptr;
    }
    //--------------------------------------------------------------------------
    void set_last_q_element (queue_prepared& uncommited_last_element)
    {
        assert (m_state == request_q_last);
        m_last_q_elem = uncommited_last_element;
        set_state (sent_q_last);
    }
    //--------------------------------------------------------------------------
    bool has_failed()  const
    {
        return (get_state() == failed);
    }
    //--------------------------------------------------------------------------
    void set_failed()
    {
        set_state (failed);
    }
    //--------------------------------------------------------------------------
private:
    //--------------------------------------------------------------------------
    enum state {
        unitialized,
        request_q_last,
        sent_q_last,
        failed,
    };
    friend class backoff_wait;
    //--------------------------------------------------------------------------
    state get_state() const
    {
        return m_state.load (mo_acquire);
    }
    //--------------------------------------------------------------------------
    void set_state (state s)
    {
        m_state.store (s, mo_release);
    }
    //--------------------------------------------------------------------------
    queue_prepared    m_last_q_elem;
    at::atomic<state> m_state;
};
//------------------------------------------------------------------------------
class backoff_wait {
public:
    //--------------------------------------------------------------------------
    void producer_push_ticket (backoff_ticket& t)
    {
        t.m_state = backoff_ticket::request_q_last;
        m_fifo.push (t);
    }
    //--------------------------------------------------------------------------
    backoff_ticket* get_next_ticket()
    {
retry:
        auto v = m_fifo.pop();
        switch (v.error) {
        case mpsc_result::no_error:
            /*no memory handling, the ticket has to be on the stack*/
            return static_cast<backoff_ticket*> (v.node);
        case mpsc_result::empty:
            return nullptr;
        case mpsc_result::busy_try_again:
            processor_pause();
            goto retry;
        default:
            return nullptr; /*unreachable*/
            break;
        }
    }
    //--------------------------------------------------------------------------
private:
    mpsc_i_fifo m_fifo;
    //--------------------------------------------------------------------------
};
//------------------------------------------------------------------------------

} //namespace mal

#endif /*MAL_BACKOFF_WAIT_HPP_*/