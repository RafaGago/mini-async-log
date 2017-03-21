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

#ifndef MAL_LOG_ASYNC_TO_SYNC_HPP_
#define MAL_LOG_ASYNC_TO_SYNC_HPP_

#include <mal_log/util/thread.hpp>
#include <mal_log/util/chrono.hpp>
#include <mal_log/sync_point.hpp>

namespace mal {

// NOTE: if this needs to be optimized on Linux, "wait" and "notify" can be done
// using a futex directly over the state variable.
//------------------------------------------------------------------------------
class async_to_sync
{
public:
    //--------------------------------------------------------------------------
    async_to_sync()
    {
        m_cancel_all = false;
    }
    //--------------------------------------------------------------------------
    ~async_to_sync()
    {
        cancel_all();
    }
    //--------------------------------------------------------------------------
    void cancel_all()
    {
        m_lock.lock();
        m_cancel_all = true;
        m_lock.unlock();
        m_cond.notify_all();
    }
    //--------------------------------------------------------------------------
    bool wait (sync_point& sync)
    {
        th::unique_lock<th::mutex> lock (m_lock);
        try {
            auto pred = [&]() {
                return (sync.state == sync_point::touched) || m_cancel_all;
            };
            m_cond.wait (lock, pred);
            return !m_cancel_all;
        }
        catch (...) {
            return false;
        }
    }
    //--------------------------------------------------------------------------
    void notify (sync_point& sync)
    {
        th::unique_lock<th::mutex> l (m_lock);
        sync.state = sync_point::touched;
        m_cond.notify_all();
    }
    //--------------------------------------------------------------------------
private:
    th::condition_variable_any m_cond;
    th::mutex                  m_lock;
    bool                       m_cancel_all;
    //--------------------------------------------------------------------------
};
//------------------------------------------------------------------------------

} //namespaces

#endif /* MAL_LOG_ASYNC_TO_SYNC_HPP_ */
