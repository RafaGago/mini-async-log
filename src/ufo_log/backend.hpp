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

#ifndef UFO_LOG_BACKEND_HPP_
#define UFO_LOG_BACKEND_HPP_

#include <cstring>
#include <cassert>
#include <new>

#include <ufo_log/util/integer.hpp>
#include <ufo_log/util/thread.hpp>
#include <ufo_log/util/mpsc.hpp>
#include <ufo_log/util/mpsc_hybrid_wait.hpp>
#include <ufo_log/util/spmc.hpp>
#include <ufo_log/util/atomic.hpp>
#include <ufo_log/util/chrono.hpp>

#include <ufo_log/output.hpp>
#include <ufo_log/frontend.hpp>
#include <ufo_log/serialization/importer.hpp>
#include <ufo_log/allocator.hpp>
#include <ufo_log/backend_cfg.hpp>
#include <ufo_log/log_files.hpp>
#include <ufo_log/timestamp.hpp>

namespace ufo {

namespace th = UFO_THREAD_NAMESPACE;
namespace ch = UFO_CHRONO_NAMESPACE;
namespace at = UFO_ATOMIC_NAMESPACE;

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

        m_cfg.blocking                         = m_wait.get_cfg();

        m_status                               = constructed;
        m_alloc_fault                          = 0;
    }
    //--------------------------------------------------------------------------
    ~backend_impl()
    {
        on_termination();
    }
    //--------------------------------------------------------------------------
    uint8* allocate_entry (uword size)
    {
        if (size)
        {
            void* mem = m_alloc.allocate (node::strict_total_size (size));
            if (mem)
            {
                return ((node*) mem)->storage();
            }
            else
            {
                m_alloc_fault.fetch_add (1, mo_relaxed);
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
        if (!m_status.compare_exchange_strong (exp, initializing, mo_relaxed))
        {
            assert (false && "log: already initialized");
            return false;
        }

        if (!alloc_init (c.alloc))
        {
            assert (false && "allocator initialization failed");
            return false;
        }

        if (!m_files.init(
                c.file.rotation.file_count + c.file.rotation.delayed_file_count,
                c.file.out_folder,
                c.file.name_prefix,
                c.file.name_suffix,
                c.file.rotation.past_files
                ))
        {
            m_alloc.free();
            return false;
        }

        auto rollback_cfg = m_cfg;
        set_cfg (c);

        m_status.store (initialized, mo_release);                               // I guess that all Kernels do this for me when launching a thread, just being on the safe side in case is not true
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
        m_wait.set_cfg (c.blocking);
    }
    //--------------------------------------------------------------------------
    bool validate_cfg (const backend_cfg& c)
    {
        auto& a = c.alloc;
        if (((!a.fixed_size_entry_count || !a.fixed_size_entry_size)) &&
              !a.use_heap_if_required
            )
        {
            assert (false && "alloc cfg invalid");
            return false;
        }
        if (a.fixed_size_entry_count && a.fixed_size_entry_size)
        {
            auto sz      = node::best_effective_size (a.fixed_size_entry_size);
            auto node_sz = node::best_total_size (sz);
            if (node_sz < sz)
            {
                assert (false && "alloc fixed size entry size would overflow");
                return false;
            }
        }
        if (c.file.rotation.file_count != 0 && c.file.aprox_size == 0)
        {
            assert (false && "won't be able to rotate infinite size files");
            return false;
        }
        if (c.file.rotation.file_count == 1)
        {
            assert (false && "won't be able to rotate a single file");
            return false;
        }
        if (c.file.out_folder.size() == 0)
        {
            assert (false && "log folder can't be empty");
            return false;
        }
        if (!m_files.can_write_in_folder (c.file.out_folder))
        {
            return false;
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
        while (m_status.load (mo_acquire) != initialized)                       // I guess that all Kernels do this for me when launching a thread, just being on the safe side in case is not true
        {
            th::this_thread::yield();
        }
        m_status.store (running, mo_relaxed);
        uword alloc_fault = m_alloc_fault.load (mo_relaxed);
        auto  next_flush  = ch::steady_clock::now() + ch::milliseconds (1000);
        new_file_name_to_buffer();

        while (true)
        {
            mpsc_result res = m_log_queue.pop();
            if (res.error == mpsc_result::no_error)
            {
                while (!non_idle_newfile_if())
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
            uword allocf_now = m_alloc_fault.load (mo_relaxed);
            if (alloc_fault != allocf_now)
            {
                write_alloc_fault (allocf_now - alloc_fault);
                alloc_fault = allocf_now;
            }
        }
        idle_rotate_if();
        m_status.store (thread_stopped, mo_relaxed);
    }
    //--------------------------------------------------------------------------
    const char* new_file_name_to_buffer()
    {
        using namespace ch;
        u64 cpu      = get_timestamp();
        u64 calendar = duration_cast<microseconds>(
                        system_clock::now().time_since_epoch()
                        ).count();
        return m_files.new_filename_in_buffer (cpu, calendar);
    }
    //--------------------------------------------------------------------------
    void write_message (mpsc_node_hook& hook)
    {
        m_decoder.import (m_out, ((node&) hook).storage());
    }
    //--------------------------------------------------------------------------
    void write_alloc_fault (uword count)
    {
        char str[96];
        std::snprintf(
                str,
                sizeof str,
                "[%020llu] [logger_err] %u alloc faults detected",
                get_timestamp(),
                count
                );
        str[sizeof str - 1] = 0;
        m_out.raw_write (sev::error, str);
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
        return (slices_files() && m_files.rotates());
    }
    //--------------------------------------------------------------------------
    bool slice_file()
    {
        assert (slices_files());
        m_out.file_close();
        return m_out.file_open (new_file_name_to_buffer());
    }
    //--------------------------------------------------------------------------
    bool reopen_file()
    {
        assert (!slices_files());
        m_out.file_close();
        return m_out.file_open (m_files.filename_in_buffer());
    }
    //--------------------------------------------------------------------------
    bool non_idle_newfile_if()
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
                        m_files.keep_newer_files(
                            m_cfg.file.rotation.file_count +
                            m_cfg.file.rotation.delayed_file_count - 1
                            );
                        m_files.push_filename_in_buffer();
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
    void idle_rotate_if()
    {
        if (rotates())
        {
            m_files.keep_newer_files (m_cfg.file.rotation.file_count);
        }
    }
    //--------------------------------------------------------------------------
    enum status
    {
        constructed,
        initializing,
        initialized,
        running,
        terminating,
        thread_stopped,
    };
    //--------------------------------------------------------------------------
    output            m_out;
    ser::importer     m_decoder;
    backend_cfg       m_cfg;
    log_files         m_files;
    th::thread        m_log_thread;
    at::atomic<uword> m_status;
    ufo_allocator     m_alloc;
    mpsc_hybrid_wait  m_wait;
    mpsc_i_fifo       m_log_queue;
    atomic_uword      m_alloc_fault;
 };
//------------------------------------------------------------------------------
} //namespaces

#endif /* UFO_LOG_BACKEND_DEF_HPP_ */
