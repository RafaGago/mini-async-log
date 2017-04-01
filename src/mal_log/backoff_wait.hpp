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
#include <mal_log/util/processor_pause.hpp>
#include <stdlib.h>

namespace mal {

class backoff_wait;
//------------------------------------------------------------------------------
class backoff_wait_ticket : public mpsc_node_hook {
public:
    //--------------------------------------------------------------------------
    backoff_wait_ticket()
    {
        m_ready = false;
        m_unlocked.store (false, at::memory_order_relaxed);
    }
    //--------------------------------------------------------------------------
    bool is_ready() {
        if (!m_ready) {
            m_ready = m_unlocked.exchange (false, at::memory_order_relaxed);
        }
        return m_ready;
    }
    //--------------------------------------------------------------------------
private:
    friend class backoff_wait;
    //--------------------------------------------------------------------------
    void set_ready() {
        m_unlocked.store (true, at::memory_order_relaxed);
    }
    //--------------------------------------------------------------------------
    at::atomic<bool> m_unlocked;
    bool             m_ready;
};
//------------------------------------------------------------------------------
class backoff_wait {
public:
    //--------------------------------------------------------------------------
    void push_ticket (backoff_wait_ticket& t)
    {
        m_fifo.push (t);
    }
    //--------------------------------------------------------------------------
    bool call_next_ticket()
    {
        backoff_wait_ticket* t;
retry:
        auto v = m_fifo.pop();
        switch (v.error) {
        case mpsc_result::no_error:
            t = static_cast<backoff_wait_ticket*> (v.node);
            t->set_ready();
            /*no memory handling, the ticket has to be on the stack*/
            return true;
        case mpsc_result::empty:
            return false;
        case mpsc_result::busy_try_again:
            processor_pause();
            goto retry;
        default:
            return false; /*unreachable*/
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