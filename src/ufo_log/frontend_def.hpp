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

#ifndef UFO_LOG_LOG_FRONTEND_DEF_HPP_
#define UFO_LOG_LOG_FRONTEND_DEF_HPP_

#include <ufo_log/util/atomic.hpp>
#include <ufo_log/util/thread.hpp>
#include <ufo_log/frontend.hpp>
#include <ufo_log/backend.hpp>

namespace ufo {

namespace th = UFO_THREAD_NAMESPACE;
namespace at = UFO_ATOMIC_NAMESPACE;

//------------------------------------------------------------------------------
class frontend::frontend_impl
{
public:
    //--------------------------------------------------------------------------
    frontend_impl()
    {
        m_state       = no_init;
        m_min_severity = sev::notice;
    }
    //--------------------------------------------------------------------------
    ~frontend_impl() {}
    //--------------------------------------------------------------------------
    ser::exporter get_encoder (uword required_bytes)
    {
        assert (m_state.load (mo_relaxed) == init);
        ser::exporter e;
        uint8* mem = m_back.allocate_entry (required_bytes);
        if (mem)
        {
            e.init (mem, required_bytes);
        }
        return e;
    }
    //--------------------------------------------------------------------------
    void push_encoded (ser::exporter encoder)
    {
        assert (m_state.load (mo_relaxed) == init);
        if (encoder.has_memory())
        {
            uint8* mem = encoder.get_memory();
            m_back.push_allocated_entry (mem);
        }
        else
        {
            assert (false && "bug!");
        }
    }
    //--------------------------------------------------------------------------
    backend_cfg get_backend_cfg() const
    {
        return m_back.get_cfg();
    }
    //--------------------------------------------------------------------------
    frontend::init_status init_backend (const backend_cfg& cfg)
    {
        uword actual = no_init;
        if (m_state.compare_exchange_strong (actual, on_init, mo_relaxed))
        {
            bool ok = m_back.init (cfg);
            m_state.store ((ok) ? init : no_init, mo_relaxed);
            return (ok) ? frontend::init_ok : frontend::init_tried_but_failed;
        }
        switch (actual)
        {
        case on_init:
        {
            uword actual = on_init;
            while (actual == on_init);
            {
                th::this_thread::yield();
                actual = m_state.load (mo_relaxed);
            }
            return (actual == init) ?
                    frontend::init_done_by_other : frontend::init_other_failed;
        }
        case init:
            return frontend::init_done_by_other;
        case terminated:
            return frontend::init_was_terminated;
        default:
            assert (false && "unreachable");
            return frontend::init_was_terminated;
        }
    }
    //--------------------------------------------------------------------------
    void on_termination()
    {
        uword actual = init;
        if (m_state.compare_exchange_strong (actual, terminated, mo_relaxed))
        {
            m_back.on_termination();
        }
    }
    //--------------------------------------------------------------------------
    bool set_console_severity (sev::severity stderr, sev::severity stdout)
    {
        if ((stdout >= sev::invalid || stderr >= sev::invalid))
        {
            assert (false);
            return false;
        }
        else if ((stderr <= stdout) && (stdout != sev::off))
        {
            assert (false);
            return false;
        }
        m_back.set_console_severity (stderr, stdout);
        m_min_severity = m_back.min_severity();
        return true;
    }
    //--------------------------------------------------------------------------
    void set_file_severity (sev::severity s)
    {
        assert (s < sev::invalid);
        m_back.set_file_severity (s);
        m_min_severity = m_back.min_severity();
    }
    //--------------------------------------------------------------------------
    sev::severity min_severity()
    {
        return (sev::severity) m_min_severity.val();
    }
    //--------------------------------------------------------------------------
private:

    enum state
    {
        no_init,
        on_init,
        init,
        terminated
    };

    backend_impl             m_back;
    mo_relaxed_atomic<uword> m_min_severity;
    mo_relaxed_atomic<uword> m_state;
};
//------------------------------------------------------------------------------
frontend::frontend() : m (new frontend::frontend_impl())
{
}

frontend::~frontend()
{
}

ser::exporter  frontend::get_encoder (uword required_bytes)
{
    return m->get_encoder (required_bytes);
}

void frontend::push_encoded (ser::exporter  encoder)
{
    m->push_encoded (encoder);
}

backend_cfg frontend::get_backend_cfg()
{
    return m->get_backend_cfg();
}

frontend::init_status frontend::init_backend (const backend_cfg& cfg)
{
    return m->init_backend (cfg);
}

sev::severity frontend::min_severity()
{
    return m->min_severity();
}

void frontend::set_file_severity (sev::severity s)
{
    return m->set_file_severity (s);
}

bool frontend::set_console_severity(
           sev::severity stderr, sev::severity stdout
           )
{
    return m->set_console_severity (stderr, stdout);
}

void frontend::on_termination()
{
    return m->on_termination();
}

} //namespace

#endif /* UFO_LOG_LOG_FRONTEND_DEF_HPP_ */
