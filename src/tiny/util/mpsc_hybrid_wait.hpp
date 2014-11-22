/*
 * mpsc_hybrid_wait.hpp
 *
 *  Created on: Nov 22, 2014
 *      Author: rafa
 */

#ifndef TINY_MPSC_HYBRID_WAIT_HPP_
#define TINY_MPSC_HYBRID_WAIT_HPP_

#include <tiny/util/integer.hpp>
#include <tiny/util/atomic.hpp>
#include <tiny/util/chrono.hpp>
#include <tiny/util/thread.hpp>

namespace tiny {

namespace th = TINY_THREAD_NAMESPACE;
namespace ch = TINY_CHRONO_NAMESPACE;
namespace at = TINY_ATOMIC_NAMESPACE;

//------------------------------------------------------------------------------
struct mpsc_hybrid_wait_cfg
{
    uword spin_max;
    uword yield_max;
    uword block_us;
    bool  never_block;
};
//------------------------------------------------------------------------------
// This class implements an hybrid lock to work with a lock-free queue. It is
// to be used under the next assumtions:
//
// -There is just a (worker) thread that waits (calls block() and reset()).
// -Many threads can call unblock().
// -The unblock() call is better to be avoided when you are sure that the worker
//  is unblocked.
//
// If low latency in the worker (at the expense of some CPU) is needed set the
// "never_block" parameter
//------------------------------------------------------------------------------
class mpsc_hybrid_wait
{
public:
    //--------------------------------------------------------------------------
     mpsc_hybrid_wait()
    {
        m_cfg.spin_max    = 50000;
        m_cfg.yield_max   = 200;
        m_cfg.never_block = false;
        m_cfg.block_us    = 20000;
        reset();
    }
    //--------------------------------------------------------------------------
    void reset()
    {
        m_spins  = 0;
        m_yields = 0;
        m_state  = unblocked;
    }
    //--------------------------------------------------------------------------
    void unblock()
    {
        m_state = unblocked;
        m_cond.notify_one();
    }
    //--------------------------------------------------------------------------
    bool block()
    {
        if (m_spins < m_cfg.spin_max)
        {
            ++m_spins;
            return false;
        }
        else if (m_yields < m_cfg.yield_max)
        {
            m_yields += ((!m_cfg.never_block) ? 1 : 0);
            th::this_thread::yield();
            return false;
        }
        else
        {
            auto pred = [&]() -> bool
            {
                uword prev = m_state.exchange ((uword) blocked, mo_relaxed);
                return prev == blocked;
            };
            m_state            = blocked;
            bool still_blocked = m_cond.wait_for(                                 //note that it's possible that the worker misses a notification and waits the whole timeout, this is by design and preferable to mutex locking the callers. if you can't tolerate this latency in the worker you can throw CPU at it by setting cfg never_block
                                    m_dummy_mutex,
                                    ch::microseconds (m_cfg.block_us),
                                    pred
                                    );
            m_state            = unblocked;
            return still_blocked;
        }
    }
    //--------------------------------------------------------------------------
    bool would_block_now_hint() const                                           //can give false positives
    {
        return !never_blocks() &&
               (m_spins < m_cfg.spin_max) &&
               (m_yields < m_cfg.yield_max);
    }
    //--------------------------------------------------------------------------
    bool never_blocks() const
    {
        return m_cfg.never_block;
    }
    //--------------------------------------------------------------------------
    mpsc_hybrid_wait_cfg get_cfg() const
    {
        return m_cfg;
    }
    //--------------------------------------------------------------------------
    void set_cfg (const mpsc_hybrid_wait_cfg& cfg)
    {
        m_cfg           = cfg;
        m_cfg.yield_max = (m_cfg.never_block) ? (uword) -1 : m_cfg.yield_max;
    }
    //--------------------------------------------------------------------------
private:
    //--------------------------------------------------------------------------
    enum state
    {
        unblocked,
//        will_block,
        blocked,
    };
    //--------------------------------------------------------------------------
    th::mutex                  m_dummy_mutex;
    th::condition_variable_any m_cond;
    mpsc_hybrid_wait_cfg       m_cfg;
    uword                      m_spins;
    uword                      m_yields;
    at::atomic<uword>          m_state;
};
 //-----------------------------------------------------------------------------
} //namespace tiny

#endif /* TINY_MPSC_HYBRID_WAIT_HPP_ */
