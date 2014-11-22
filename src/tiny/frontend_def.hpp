/*
 * log_frontend_def.hpp
 *
 *  Created on: Nov 14, 2014
 *      Author: rafgag
 */


#ifndef TINY_LOG_LOG_FRONTEND_DEF_HPP_
#define TINY_LOG_LOG_FRONTEND_DEF_HPP_

#include <tiny/util/atomic.hpp>
#include <tiny/util/thread.hpp>
#include <tiny/frontend.hpp>
#include <tiny/backend.hpp>

namespace tiny {

namespace th = TINY_THREAD_NAMESPACE;
namespace at = TINY_ATOMIC_NAMESPACE;

//------------------------------------------------------------------------------
class frontend::frontend_impl
{
public:
    //--------------------------------------------------------------------------
    frontend_impl()
    {
        m_state    = no_init;
        m_severity = sev::notice;
    }
    //--------------------------------------------------------------------------
    ~frontend_impl() {}
    //--------------------------------------------------------------------------
    proto::encoder get_encoder (uword required_bytes)
    {
        assert (m_state.load (mo_relaxed) == init);
        proto::encoder m;
        uint8* mem = m_back.allocate_entry (required_bytes);
        if (mem)
        {
            m.init (mem, required_bytes);
        }
        return m;
    }
    //--------------------------------------------------------------------------
    void push_encoded (proto::encoder encoder)
    {
        assert (m_state.load (mo_relaxed) == init);
        if (encoder.can_encode())
        {
            uint8* mem = encoder.get_result();
            m_back.push_allocated_entry (mem);
        }
        else
        {
            assert (false && "bug!");
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
        return true;
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
    void set_severity (sev::severity s)
    {
        assert (severity() < sev::invalid);
        m_severity.store (s, mo_relaxed);
    }
    //--------------------------------------------------------------------------
    sev::severity severity()
    {
        return (sev::severity) m_severity.load (mo_relaxed);
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

    backend_impl      m_back;
    at::atomic<uword> m_severity;
    at::atomic<uword> m_state;
};
//------------------------------------------------------------------------------
frontend::frontend() : m (new frontend::frontend_impl())
{
}

frontend::~frontend()
{
}

proto::encoder frontend::get_encoder (uword required_bytes)
{
    return m->get_encoder (required_bytes);
}

void frontend::push_encoded (proto::encoder encoder)
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

void frontend::set_severity (sev::severity s)
{
    return m->set_severity (s);
}

sev::severity frontend::severity()
{
    return m->severity();
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

#endif /* TINY_LOG_LOG_FRONTEND_DEF_HPP_ */
