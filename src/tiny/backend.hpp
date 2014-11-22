/*
 * log_backend.hpp
 *
 *  Created on: Nov 11, 2014
 *      Author: rafgag
 */


#ifndef TINY_LOG_BACKEND_HPP_
#define TINY_LOG_BACKEND_HPP_

#include <cstring>
#include <cassert>
#include <cstdio>
#include <fstream>
#include <new>

#include <tiny/util/integer.hpp>
#include <tiny/util/thread.hpp>
#include <tiny/util/mpsc.hpp>
#include <tiny/util/mpsc_hybrid_wait.hpp>
#include <tiny/util/spmc.hpp>
#include <tiny/util/atomic.hpp>
#include <tiny/util/chrono.hpp>
#include <tiny/output.hpp>
#include <tiny/frontend.hpp>
#include <tiny/message_decode_and_fwd.hpp>
#include <tiny/allocator.hpp>
#include <tiny/backend_cfg.hpp>

namespace tiny {

namespace th = TINY_THREAD_NAMESPACE;
namespace ch = TINY_CHRONO_NAMESPACE;
namespace at = TINY_ATOMIC_NAMESPACE;

//------------------------------------------------------------------------------
struct node : public mpsc_node_hook
{
    u8* storage()
    {
        return ((u8*) this) + sizeof *this;
    }

    static node* from_storage (u8* mem)
    {
        return (node*) (mem - sizeof (node));
    }

    static uword strict_total_size (uword effective_size)
    {
        return sizeof (node) + effective_size;
    }

    static u16 best_total_size (u16 effective_size)
    {
        static const u16 align = std::alignment_of<node>::value;
        u16 node_sz = ((sizeof (node) + effective_size + align - 1) / align);
        return node_sz * align;
    }

    static u16 optimize_effective_size (u16 effective_size)
    {
        return best_total_size (effective_size) - sizeof (node);
    }

    static u16 best_effective_size (u16 max_total_size)
    {
        static const u16 align = std::alignment_of<node>::value;
        u16 candidate          = max_total_size - sizeof (node);
        u16 res                = best_total_size (candidate);
        if (res <= max_total_size)
        {
            return candidate;
        }
        else
        {
            candidate -= align;
            res        = best_total_size (candidate);
            return res - sizeof (node);
        }
    }
};
//------------------------------------------------------------------------------
class backend_impl
{
public:
    //--------------------------------------------------------------------------
    backend_impl()
    {
        m_cfg.alloc.use_heap_if_required       = true;
        m_cfg.alloc.fixed_size_entry_size      = 128;
        m_cfg.alloc.fixed_size_entry_count     = 1024;

        m_cfg.display.show_severity            = m_decoder.prints_severity;
        m_cfg.display.show_timestamp           = m_decoder.prints_timestamp;

        m_cfg.file.aprox_size                  = 1024;
        m_cfg.file.name_suffix                 = ".log";
        m_cfg.file.rotation.file_count         = 0;
        m_cfg.file.rotation.delayed_file_count = 0;

        m_status                               = constructed;
    }
    //--------------------------------------------------------------------------
    ~backend_impl()
    {
        on_termination();
    }
    //--------------------------------------------------------------------------
    uint8* allocate_entry (uword size)
    {
        if (size && size <= proto::largest_message_bytesize)
        {
            void* mem = m_alloc.allocate (node::strict_total_size (size));
            if (mem)
            {
                return ((node*) mem)->storage();
            }
            else
            {
                m_overflow.fetch_add (1, mo_relaxed);
                return nullptr;
            }
        }
        else
        {
            assert (false && "invalid size");
            return nullptr;
        }
    }
    //--------------------------------------------------------------------------
    void push_allocated_entry (uint8* entry)
    {
        node* n             = node::from_storage (entry);
        bool was_empty_hint = m_log_queue.push (*n);
        if (was_empty_hint && !m_wait.never_blocks())
        {
            m_wait.unblock();
        }
    }
    //--------------------------------------------------------------------------
    void set_console_severity (sev::severity stderr, sev::severity stdout)
    {
        m_out.set_console_severity (stderr, stdout);
    }
    //--------------------------------------------------------------------------
    backend_cfg get_cfg() const
    {
        return m_cfg;
    }
    //--------------------------------------------------------------------------
    bool init (const backend_cfg& c)
    {
        if (!validate_cfg (c)) { return false; }

        uword exp = constructed;
        if (!m_status.compare_exchange_strong (exp, initialized, mo_relaxed))
        {
            assert (false && "log: already initialized");
            return false;
        }

        if (!alloc_init (c.alloc))
        {
            assert (false && "allocator initialization failed");
            return false;
        }

        auto rollback_cfg = m_cfg;
        set_cfg (c);

        m_log_thread = th::thread ([this](){ this->thread(); });

        while (m_status.load (mo_relaxed) == initialized)
        {
            th::this_thread::yield();
        }
        if (m_status.load (mo_relaxed) == running)
        {
            return true;
        }
        else
        {
            set_cfg (rollback_cfg);
            return false;
        }
    }
    //--------------------------------------------------------------------------
    void on_termination()
    {
        m_status.store (terminating, mo_relaxed);
        m_log_thread.join();
    }
    //--------------------------------------------------------------------------
private:
    void set_cfg (const backend_cfg& c)
    {
        m_cfg = c;
        m_decoder.prints_severity  = m_cfg.display.show_severity;
        m_decoder.prints_timestamp = m_cfg.display.show_timestamp;
    }
    //--------------------------------------------------------------------------
    static bool validate_cfg (const backend_cfg& c)
    {
        auto& a = c.alloc;
        if (((!a.fixed_size_entry_count || !a.fixed_size_entry_size)) &&
              !a.use_heap_if_required
            )
        {
            assert (false && "alloc cfg invalid");
            return false;
        }

        auto sz      = node::best_effective_size (a.fixed_size_entry_size);
        auto node_sz = node::best_total_size (sz);
        if (node_sz < sz)
        {
            assert (false && "alloc fixed size entry size would overflow");
            return false;
        }

        if (c.file.rotation.file_count != 0 && c.file.aprox_size == 0)
        {
            assert (false && "won't be able to rotate infinite size files");
            return false;
        }

        if (c.file.out_folder.size() == 0)
        {
            assert (false && "log folder can't be empty");
            return false;
        }

        for (uword i = 0; ; ++i)
        {
            std::string name = new_file_name (c.file);
            std::ofstream file (name.c_str());
            file.write ((const char*) &name, 30);
            if (file.good())
            {
                file.close();
                erase_file (name.c_str());
                break;
            }
            erase_file (name.c_str());
            if (i == 5) //FIXME
            {
                assert (false && "log: couldn't create or write a test file");
                return false;
            }
            th::this_thread::sleep_for (ch::milliseconds (1));
        }
        return true;
    }
    //--------------------------------------------------------------------------
    bool alloc_init (const backend_log_entry_alloc_config& c)
    {
        auto sz = node::best_total_size(
                      node::best_effective_size (c.fixed_size_entry_size)
                        );
        return m_alloc.init(
                    c.fixed_size_entry_count,
                    c.fixed_size_entry_size ? sz : 0,
                    c.use_heap_if_required
                    );
    }
    //--------------------------------------------------------------------------
    void thread()
    {
        m_status.store (running, mo_relaxed);
        uword overflow_past = m_overflow.load (mo_relaxed);
        auto  next_flush    = ch::steady_clock::now() + ch::milliseconds (1000);

        while (true)
        {
            mpsc_result res = m_log_queue.pop();
            if (res.error == mpsc_result::no_error)
            {
                while (!newfile_if())
                {
                    th::this_thread::sleep_for (ch::milliseconds (1));
                    //how to and when to notify this???
                }
                m_wait.reset();
                write_message (*res.node);
                m_alloc.deallocate ((void*) res.node);
            }
            else if (res.error == mpsc_result::empty)
            {
                if (m_status.load (mo_relaxed) != running)
                {
                    break;
                }
                if (m_wait.would_block_now_hint())
                {
                    idle_rotate_if();
                    auto now = ch::steady_clock::now();
                    if (now >= next_flush)
                    {
                        next_flush = now + ch::milliseconds (1000);
                        m_out.flush();
                    }
                }
                m_wait.block();
            }
            else
            {
                for (uword i = 0; i < 1000; ++i);                               //standard pause instruction to burn cycles without using battery? x86 has it...
                continue;
            }
            uword overflow_new = m_overflow.load (mo_relaxed);
            if (overflow_past != overflow_new)
            {
                write_overflow (overflow_new - overflow_past);
                overflow_past = overflow_new;
            }
        }
        idle_rotate_if();
        m_status.store (thread_stopped, mo_relaxed);
    }
    //--------------------------------------------------------------------------
    static std::string new_file_name(
            const backend_file_config& cfg
            )
    {
        using namespace ch;
        u64 tstamp   = get_tstamp ();
        u64 calendar = duration_cast<microseconds>(
                        system_clock::now().time_since_epoch()
                        ).count();

        std::string name (cfg.out_folder);
        name.append (cfg.name_prefix);                                          //A C++11 reserve is possible

        char buff[16 + 1 + 16 + 1];
        snprintf (buff, sizeof buff, "%016llx-%016llx", calendar, tstamp);
        buff[sizeof buff - 1] = 0;

        name.append (buff);
        if (cfg.name_suffix.size())
        {
            name.append (cfg.name_suffix);
        }
        return name;
    }
    //--------------------------------------------------------------------------
    static u64 get_tstamp ()
    {
        using namespace TINY_CHRONO_NAMESPACE;
        return duration_cast<microseconds>(
                steady_clock::now().time_since_epoch()
                ).count();
    }
    //--------------------------------------------------------------------------
    void write_message (mpsc_node_hook& hook)
    {
        m_decoder.new_entry (((node&) hook).storage());
        if (m_decoder.has_content())
        {
            m_decoder.decode_and_fwd_entry (m_out, get_tstamp());
        }
    }
    //--------------------------------------------------------------------------
    void write_overflow (uword count)
    {
        m_decoder.fwd_overflow_entry (m_out, get_tstamp(), count);
    }
    //--------------------------------------------------------------------------
    bool slices_files() const
    {
        return (m_cfg.file.aprox_size != 0);
    }
    //--------------------------------------------------------------------------
    bool has_to_slice_now()
    {
        return (m_out.file_bytes_written() >= m_cfg.file.aprox_size);
    }
    //--------------------------------------------------------------------------
    bool rotates() const
    {
        return (slices_files() && m_cfg.file.rotation.file_count != 0);
    }
    //--------------------------------------------------------------------------
    bool slice_file()
    {
        assert (slices_files());
        m_out.file_close();
        auto name = new_file_name (m_cfg.file);
        if (m_out.file_open (name.c_str()))
        {
            m_cfg.file.rotation.past_files.push_back (name);
            return true;
        }
        return false;
    }
    //--------------------------------------------------------------------------
    bool reopen_file()
    {
        assert (!slices_files());
        m_out.file_close();

        auto& pf = m_cfg.file.rotation.past_files;
        if (!pf.size())
        {
            pf.push_back (new_file_name (m_cfg.file));
        }
        return m_out.file_open (pf.back().c_str());
    }
    //--------------------------------------------------------------------------
    bool newfile_if()
    {
        bool error = !m_out.file_is_open() || !m_out.file_no_error();
        if (slices_files())
        {
            if (has_to_slice_now() || error)
            {
                auto success = slice_file();
                if (success)
                {
                    if (rotates())
                    {
                        rotate(
                            m_cfg.file.rotation.file_count +
                            m_cfg.file.rotation.delayed_file_count
                            );
                    }
                    else
                    {
                        m_cfg.file.rotation.past_files.clear();
                    }
                }
                return success;
            }
        }
        else if (error)
        {
            return reopen_file();
        }
        return true;
    }
    //--------------------------------------------------------------------------
    void rotate (uword limit)
    {
        assert (rotates());
        auto& pf = m_cfg.file.rotation.past_files;
        while (pf.size() > limit)
        {
            erase_file (pf.front().c_str());
            pf.pop_front();
        }
    }
    //--------------------------------------------------------------------------
    void idle_rotate_if()
    {
        if (rotates()) { rotate (m_cfg.file.rotation.file_count); }
    }
    //--------------------------------------------------------------------------
    static void erase_file (const char* file)
    {
        std::remove (file);
    }
    //--------------------------------------------------------------------------
    enum status
    {
        constructed,
        initialized,
        running,
        terminating,
        thread_stopped,
    };
    //--------------------------------------------------------------------------
    output                     m_out;
    proto::decode_and_fwd      m_decoder;
    backend_cfg                m_cfg;
    past_executions_file_list  m_past_files;
    th::thread                 m_log_thread;
    at::atomic<uword>          m_status;
    tiny_allocator             m_alloc;
    mpsc_hybrid_wait           m_wait;
    mpsc_i_fifo                m_log_queue;
    atomic_uword               m_overflow;
 };
//------------------------------------------------------------------------------

} //namespaces

#endif /* TINY_LOG_BACKEND_DEF_HPP_ */
