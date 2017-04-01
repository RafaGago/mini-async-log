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

#ifndef MAL_LOG_LOG_FRONTEND_DEF_HPP_
#define MAL_LOG_LOG_FRONTEND_DEF_HPP_

#include <utility>
#include <cassert>
#include <cstdlib>
#include <mal_log/util/atomic.hpp>
#include <mal_log/util/thread.hpp>
#include <mal_log/util/queue_backoff.hpp>
#include <mal_log/frontend.hpp>
#include <mal_log/backend.hpp>
#include <mal_log/async_to_sync.hpp>
#include <mal_log/timestamp.hpp>


namespace mal {

//------------------------------------------------------------------------------
class frontend::frontend_impl
{
public:
    //--------------------------------------------------------------------------
    frontend_impl()
    {
        m_state               = no_init;
        m_min_severity        = sev::notice;
        m_prints_timestamp    = true;
        m_producer_timestamp  = true;
        m_timestamp_base      = 0;
        m_block_on_full_queue = false;
        srand ((unsigned int) get_timestamp() >> 2);
    }
    //--------------------------------------------------------------------------
    ~frontend_impl() {}
    //--------------------------------------------------------------------------
    ser::exporter get_encoder (uword required_bytes)
    {
        ser::exporter e;
        typedef decltype (e.opaque_data) od;                                    //visual studio 2010 doesn't like this inside the static assertion
        static_assert(
            od::bytes >= sizeof (queue_prepared), "not enough opaque storage"
            );
        assert(
            m_state.load (mo_relaxed) == init &&
            "using the logger in a non-initialized state"
            );
        queue_prepared commit_data = m_back.allocate_entry (required_bytes);
        u8*                   mem  = commit_data.get_mem();
        queue_prepared::error err  = commit_data.get_error();

        if (!mem && m_block_on_full_queue && err == queue_prepared::queue_full){
            th::mutex dummy;
            cond_queue_backoff backoff (dummy, m_back.consume_condition);
            backoff.cfg.long_sleep_ns = 20000000;
            do {

                backoff.wait();
                commit_data = m_back.allocate_entry (required_bytes);
                mem         = commit_data.get_mem();
                err         = commit_data.get_error();
            }
            while (!mem && err == queue_prepared::queue_full);
        }
        if (mem) {
            e.init (mem, required_bytes);
            e.opaque_data.write (commit_data);
        }
        return e;
    }
    //--------------------------------------------------------------------------
    bool sync_push_encoded(
            ser::exporter& encoder, sync_point& sync
            )
    {
        assert (m_state.load (mo_relaxed) == init);
        if (encoder.has_memory()) {
            m_back.push_entry (encoder.opaque_data.read_as<queue_prepared>());
            return m_sync.wait (sync);
        }
        else {
            assert (false && "bug!");
            return false;
        }
    }
    //--------------------------------------------------------------------------
    void async_push_encoded (ser::exporter& encoder)
    {
        assert (m_state.load (mo_relaxed) == init);
        if (encoder.has_memory()) {
            m_back.push_entry (encoder.opaque_data.read_as<queue_prepared>());
        }
        else {
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
        if (m_state.compare_exchange_strong (actual, on_init, mo_acquire)) {
            m_timestamp_base = get_timestamp();
            auto sev_ch = [=]() { this->severity_updated_event(); };
            if (m_back.init (cfg, m_sync, m_timestamp_base, sev_ch))
            {
                m_prints_timestamp = cfg.display.show_timestamp;
                m_state.store (init, mo_release);
                return frontend::init_ok;
            }
            else {
                m_state.store (no_init, mo_relaxed);
                return frontend::init_tried_but_failed;
            }
        }
        switch (actual) {
        case on_init: {
            uword actual = on_init;
            while (actual == on_init) {
                th::this_thread::yield();
                actual = m_state.load (mo_acquire);
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
    u64 timestamp_base() const
    {
        return m_timestamp_base;
    }
    //--------------------------------------------------------------------------
    void on_termination()
    {
        uword actual = init;
        if (m_state.compare_exchange_strong (actual, terminated, mo_relaxed)) {
            m_sync.cancel_all();
            m_back.on_termination();
        }
    }
    //--------------------------------------------------------------------------
    bool set_console_severity (sev::severity std_err, sev::severity std_out)
    {
        if ((std_out >= sev::invalid || std_err >= sev::invalid)) {
            assert (false);
            return false;
        }
        else if ((std_err <= std_out) && (std_out != sev::off)) {
            assert (false);
            return false;
        }
        m_back.set_console_severity (std_err, std_out);
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
    bool initialized() const
    {
        return (m_state == init);
    }
    //--------------------------------------------------------------------------
    sev::severity min_severity() const
    {
        return (sev::severity) m_min_severity.val();
    }
    //--------------------------------------------------------------------------
    bool can_log (sev::severity s) const
    {
        return initialized() && (s >= min_severity());
    }
    //--------------------------------------------------------------------------
    bool producer_timestamp() const
    {
        return m_producer_timestamp && m_prints_timestamp;
    }
    //--------------------------------------------------------------------------
    bool producer_timestamp (bool on)
    {
        m_producer_timestamp = on;
        return producer_timestamp();
    }
    //--------------------------------------------------------------------------
    void block_on_full_queue (bool on)
    {
        m_block_on_full_queue           = on;
        m_back.signal_consume_condition = on;
    }
    //--------------------------------------------------------------------------
    bool block_on_full_queue() const
    {
        return m_block_on_full_queue;
    }
    //--------------------------------------------------------------------------
private:
    //--------------------------------------------------------------------------
    void severity_updated_event()
    {
        m_min_severity = m_back.min_severity();
    }
    //--------------------------------------------------------------------------
    enum state
    {
        no_init,
        on_init,
        init,
        terminated
    };
    //--------------------------------------------------------------------------
    u64                      m_timestamp_base;
    mo_relaxed_atomic<uword> m_min_severity;
    mo_relaxed_atomic<uword> m_state;
    backend_impl             m_back;
    async_to_sync            m_sync;
    bool                     m_prints_timestamp;
    bool                     m_producer_timestamp;
    bool                     m_block_on_full_queue;
};
//------------------------------------------------------------------------------
MAL_LIB_EXPORTED_CLASS frontend::frontend()
{
    m = nullptr;
    m = new frontend::frontend_impl();
}
//------------------------------------------------------------------------------
MAL_LIB_EXPORTED_CLASS frontend::~frontend()
{
    if (is_constructed())
    {
        delete m;
    }
}
//------------------------------------------------------------------------------
bool MAL_LIB_EXPORTED_CLASS frontend::is_constructed() const
{
    return m != nullptr;
}
//------------------------------------------------------------------------------
ser::exporter MAL_LIB_EXPORTED_CLASS
    frontend::get_encoder (uword required_bytes)
{
    assert (is_constructed());
    return m->get_encoder (required_bytes);
}
//------------------------------------------------------------------------------
void MAL_LIB_EXPORTED_CLASS frontend::async_push_encoded(
        ser::exporter& encoder
        )
{
    assert (is_constructed());
    m->async_push_encoded (encoder);
}
//--------------------------------------------------------------------------
bool MAL_LIB_EXPORTED_CLASS frontend::sync_push_encoded(
        ser::exporter& encoder,
        sync_point&    sync
        )
{
    assert (is_constructed());
    return m->sync_push_encoded (encoder, sync);
}
//------------------------------------------------------------------------------
backend_cfg MAL_LIB_EXPORTED_CLASS frontend::get_backend_cfg()
{
    assert (is_constructed());
    return m->get_backend_cfg();
}
//------------------------------------------------------------------------------
frontend::init_status MAL_LIB_EXPORTED_CLASS
    frontend::init_backend (const backend_cfg& cfg)
{
    assert (is_constructed());
    return m->init_backend (cfg);
}
//------------------------------------------------------------------------------
sev::severity MAL_LIB_EXPORTED_CLASS frontend::min_severity() const
{
    assert (is_constructed());
    return m->min_severity();
}
//--------------------------------------------------------------------------
bool MAL_LIB_EXPORTED_CLASS frontend::can_log (sev::severity s) const
{
    assert (is_constructed());
    return m->can_log (s);
}
//------------------------------------------------------------------------------
void MAL_LIB_EXPORTED_CLASS frontend::set_file_severity (sev::severity s)
{
    assert (is_constructed());
    return m->set_file_severity (s);
}
//------------------------------------------------------------------------------
bool MAL_LIB_EXPORTED_CLASS frontend::set_console_severity(
           sev::severity std_err, sev::severity std_out
           )
{
    assert (is_constructed());
    return m->set_console_severity (std_err, std_out);
}
//--------------------------------------------------------------------------
timestamp_data MAL_LIB_EXPORTED_CLASS frontend::get_timestamp_data() const
{
    timestamp_data d;
    d.producer_timestamps = m->producer_timestamp();
    d.base                = m->timestamp_base();
    return d;
}
//------------------------------------------------------------------------------
bool MAL_LIB_EXPORTED_CLASS frontend::producer_timestamp (bool on)
{
    assert (is_constructed());
    return m->producer_timestamp (on);
}
//------------------------------------------------------------------------------
void MAL_LIB_EXPORTED_CLASS frontend::on_termination()
{
    assert (is_constructed());
    return m->on_termination();
}
//------------------------------------------------------------------------------
void MAL_LIB_EXPORTED_CLASS frontend::block_on_full_queue(bool on)
{
    assert (is_constructed());
    m->block_on_full_queue(on);
}
//------------------------------------------------------------------------------
bool MAL_LIB_EXPORTED_CLASS frontend::block_on_full_queue() const
{
    assert (is_constructed());
    return m->block_on_full_queue();
}
//------------------------------------------------------------------------------
} //namespace

#endif /* MAL_LOG_LOG_FRONTEND_DEF_HPP_ */
