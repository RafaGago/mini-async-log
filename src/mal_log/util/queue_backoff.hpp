/*
The BSD 3-clause license
--------------------------------------------------------------------------------
Copyright (c) 2017 Rafael Gago Castano. All rights reserved.

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
#ifndef MAL_QUEUE_BACKOFF_HPP_
#define MAL_QUEUE_BACKOFF_HPP_

#include <mal_log/timestamp.hpp>
#include <mal_log/util/thread.hpp>
#include <mal_log/util/chrono.hpp>
#include <mal_log/util/queue_backoff_cfg.hpp>
#include <mal_log/util/processor_pause.hpp>
#include <stdlib.h>

namespace mal { namespace detail {

class queue_backoff {
public:
    queue_backoff_cfg cfg;
    //--------------------------------------------------------------------------
    queue_backoff()
    {
        m_iterations             = 0;
        cfg.spin_end             = 4;
        cfg.short_cpu_relax_end  = 16;
        cfg.long_cpu_relax_end   = 32;
        cfg.yield_end            = 52;
        cfg.short_sleep_end      = 78;
        cfg.long_sleep_ns        = 8000000;
        cfg.long_sleep_randomize = true;
    }
    //--------------------------------------------------------------------------
    void reset()
    {
        m_iterations = 0;
    }
    //--------------------------------------------------------------------------
    bool next_wait_is_long_sleep()
    {
        return m_iterations + 1 >= cfg.short_sleep_end;
    }
    //--------------------------------------------------------------------------
protected:
    bool wait()
    {
        ++m_iterations;
        if (m_iterations < cfg.spin_end) {
            return true;
        }
        else if (m_iterations < cfg.short_cpu_relax_end) {
            processor_pause();
            return true;
        }
        else if (m_iterations < cfg.long_cpu_relax_end) {
            for (int i = 0; i < 7; ++i) {
                processor_pause();
                processor_pause();
                processor_pause();
                processor_pause();
                processor_pause();
            }
            return true;
        }
        if (m_iterations < cfg.yield_end) {
            th::this_thread::sleep_for (ch::nanoseconds (500));
        }
        else if (m_iterations < cfg.short_sleep_end) {
            th::this_thread::sleep_for(
                ch::nanoseconds (1500 + (rand() % 512))
                );
            return true;
        }
        return false;
    }
    //--------------------------------------------------------------------------
    uword get_sleep_block_ns()
    {
        return cfg.long_sleep_randomize ?
            cfg.long_sleep_ns + (rand() % (cfg.long_sleep_ns / 4)) :
            cfg.long_sleep_ns;
    }
    //--------------------------------------------------------------------------
    uword m_iterations;
};

} //namespace detail

//------------------------------------------------------------------------------
class sleep_queue_backoff : public detail::queue_backoff {
public:
    //--------------------------------------------------------------------------
    void wait()
    {
        if (detail::queue_backoff::wait()) {
            return;
        }
        th::this_thread::sleep_for (ch::nanoseconds (get_sleep_block_ns()));
    }
    //--------------------------------------------------------------------------
};
//------------------------------------------------------------------------------
class mutex_queue_backoff : public detail::queue_backoff {
public:
    //--------------------------------------------------------------------------
    mutex_queue_backoff (th::timed_mutex& m) :
        m_lock (m, th::defer_lock)
    {}
    //--------------------------------------------------------------------------
    void wait()
    {
        if (m_locked) {
            m_iterations = cfg.long_cpu_relax_end;
        }
        if (detail::queue_backoff::wait()) {
            return;
        }
        m_locked = m_lock.try_lock_for (ch::nanoseconds (get_sleep_block_ns()));
    }
    //--------------------------------------------------------------------------
private:
    th::unique_lock<th::timed_mutex> m_lock;
    bool                             m_locked;
};
//------------------------------------------------------------------------------
class cond_queue_backoff : public detail::queue_backoff {
public:
    //--------------------------------------------------------------------------
    cond_queue_backoff (th::mutex& m, th::condition_variable& cond) :
        m_mutex (m),
        m_cond (cond)
    {}
    //--------------------------------------------------------------------------
    void wait()
    {
        if (!detail::queue_backoff::wait()) {
            th::unique_lock<th::mutex> lock (m_mutex);
            m_cond.wait_for (lock, ch::nanoseconds (get_sleep_block_ns()));
        }
    }
    //--------------------------------------------------------------------------
    void wait (th::unique_lock<th::mutex>& lock)
    {
        if (!detail::queue_backoff::wait()) {
            m_cond.wait_for (lock, ch::nanoseconds (get_sleep_block_ns()));
        }
    }
    //--------------------------------------------------------------------------
    void notify_all()
    {
        m_cond.notify_all();
    }
    //--------------------------------------------------------------------------
    void notify_one()
    {
        m_cond.notify_all();
    }
    //--------------------------------------------------------------------------
private:
    th::mutex&              m_mutex;
    th::condition_variable& m_cond;
};
//------------------------------------------------------------------------------
} //namespace
#endif
