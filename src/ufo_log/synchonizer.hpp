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

#ifndef UFO_LOG_SYNCHONIZER_HPP_
#define UFO_LOG_SYNCHONIZER_HPP_

#include <memory>
#include <ufo_log/util/thread.hpp>
#include <ufo_log/util/chrono.hpp>

#include <ufo_log/util//atomic.hpp>
#include <ufo_log/util/mpsc_hybrid_wait.hpp>

namespace ufo {

//------------------------------------------------------------------------------
struct log_sync_resources
{
public:
    log_sync_resources()
    {
        m_signaled = false;
    }
    //--------------------------------------------------------------------------
    bool wait (uword timeout_ms)
    {
        try
        {
            auto pred = [&](){ return m_signaled; };
            th::unique_lock<boost::mutex> l (m_lock);
            if (timeout_ms)
            {
                return m_cond.wait_for(
                            l, ch::milliseconds (timeout_ms), pred
                            );
            }
            else
            {
                m_cond.wait (l, pred);
                return true;
            }
        }
        catch (...)
        {
            return false;
        }
    }
    //--------------------------------------------------------------------------
    void notify()
    {
        th::unique_lock<boost::mutex> l (m_lock);
        m_signaled = true;
        m_cond.notify_one();
    }
    //--------------------------------------------------------------------------
private:
    boost::condition_variable_any m_cond;
    boost::mutex                  m_lock;
    bool                          m_signaled;
    //--------------------------------------------------------------------------
};
//------------------------------------------------------------------------------
typedef std::shared_ptr<log_sync_resources> synchronizer;
//------------------------------------------------------------------------------
} //namespaces

#endif /* UFO_LOG_SYNCHONIZER_HPP_ */
