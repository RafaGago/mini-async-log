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

#ifndef MAL_LOG_BACKEND_HPP_
#define MAL_LOG_BACKEND_HPP_

#include <cassert>
#include <cstdio>
#include <functional>
#include <new>
#include <ostream>
#include <cstdlib>

#include <mal_log/util/integer.hpp>
#include <mal_log/util/thread.hpp>
#include <mal_log/util/atomic.hpp>
#include <mal_log/util/chrono.hpp>
#include <mal_log/util/mem_printf.hpp>
#include <mal_log/util/queue_backoff.hpp>

#include <mal_log/output.hpp>
#include <mal_log/frontend.hpp>
#include <mal_log/log_writer.hpp>
#include <mal_log/queue.hpp>
#include <mal_log/cfg.hpp>
#include <mal_log/log_file_register.hpp>
#include <mal_log/timestamp.hpp>

namespace mal {

class async_to_sync;

//------------------------------------------------------------------------------
class backend_impl
{
public:
    typedef std::function<void()> sev_update_evt;
    //--------------------------------------------------------------------------
    cfg config;
    //--------------------------------------------------------------------------
    backend_impl()
    {
        m_status             = constructed;
        m_alloc_fault        = 0;
        m_on_error_avoidance = false;
        set_cfg_defaults (config);
    }
    //--------------------------------------------------------------------------
    ~backend_impl()
    {
        on_termination();
    }
    //--------------------------------------------------------------------------
    queue_prepared allocate_entry (uword size)
    {
        if (size) {
            return m_fifo.mp_bounded_push_prepare (size);
        }
        else {
            assert (false && "invalid size");
            return queue_prepared();
        }
    }
    //--------------------------------------------------------------------------
    queue_prepared reserve_next_bounded_entry()
    {
        return m_fifo.mp_bounded_push_next_entry_reserve();
    }
    //--------------------------------------------------------------------------
    queue_prepared::error bounded_entry_is_ready (const queue_prepared& entry)
    {
        return m_fifo.mp_bounded_push_reserved_entry_is_ready (entry);
    }
    //--------------------------------------------------------------------------
    void push_entry (const queue_prepared& entry)
    {
        m_fifo.bounded_push_commit (entry);
    }
    //--------------------------------------------------------------------------
    void set_file_severity (sev::severity s)
    {
        m_out.set_file_severity (s);
    }
    //--------------------------------------------------------------------------
    void set_console_severity (sev::severity std_err, sev::severity std_out)
    {
        m_out.set_console_severity (std_err, std_out);
    }
    //--------------------------------------------------------------------------
    sev::severity min_severity() const
    {
        return m_out.min_severity();
    }
    //--------------------------------------------------------------------------
    bool init(
        const cfg&            c,
        async_to_sync&        sync,
        u64                   timestamp_base,
        const sev_update_evt& su
        )
    {
        if (!validate_cfg (c)) { return false; }

        uword exp = constructed;
        if (!m_status.compare_exchange_strong (exp, initializing, mo_relaxed)) {
            std::cerr << "[logger] already initialized\n";
            assert (false && "log: already initialized");
            return false;
        }
        uword bsz     = c.queue.bounded_q_block_size;
        uword esz     = c.queue.bounded_q_entry_size;
        uword entries = (bsz && esz) ? (bsz / esz) :  0;
        if (!m_fifo.init (bsz, entries, c.queue.can_use_heap_q)) {
            std::cerr << "[logger] queue initialization failed\n";
            assert (false && "queue initialization failed");
            return false;
        }
        if (!m_files_register.init(
                c.file.rotation.file_count + c.file.rotation.delayed_file_count,
                c.file.out_folder,
                c.file.name_prefix,
                c.file.name_suffix,
                c.file.rotation.past_files
                )) {
            m_fifo.clear();
            return false;
        }
        auto rollback_cfg = config;
        set_cfg (c);

        m_writer.set_synchronizer (sync);
        m_writer.set_timestamp_base (timestamp_base);
        m_files_register.set_timestamp_base (timestamp_base);

        m_status.store (initialized, mo_release);                               // I guess that all Kernels do this for me when launching a thread, just being on the safe side in case is not true
        m_sev_evt    = su;
        m_log_thread = th::thread ([this](){ this->thread(); });

        while (m_status.load (mo_relaxed) == initialized) {
            th::this_thread::yield();
        }
        if (m_status.load (mo_relaxed) == running) {
            return true;
        }
        else {
            set_cfg (rollback_cfg);
            return false;
        }
    }
    //--------------------------------------------------------------------------
    void on_termination()
    {
        m_fifo.block_producers();
        uword exp = running;
        if (m_status.compare_exchange_strong (exp, terminating, mo_relaxed)) {
            m_log_thread.join();
        }
    }
    //--------------------------------------------------------------------------
    bool prints_timestamp()
    {
        return m_writer.prints_timestamp;
    }
    //--------------------------------------------------------------------------
private:
    //--------------------------------------------------------------------------
    void set_cfg_defaults (cfg &c)
    {
        c.queue.can_use_heap_q         = true;
        c.queue.bounded_q_entry_size   = 64;
        c.queue.bounded_q_block_size   = 64 * 4096;
        c.queue.bounded_q_blocking_sev = sev::off;

        c.display.show_severity  = m_writer.prints_severity;
        c.display.show_timestamp = m_writer.prints_timestamp;

        c.file.aprox_size          = 1024;
        c.file.name_suffix         = ".log";
        c.file.rotation.file_count = 0;
        c.file.rotation.delayed_file_count     = 0;
        c.file.erase_and_retry_on_fatal_errors = false;

        c.consumer_backoff = m_wait.cfg;

        c.producer_backoff.spin_end            = 2;
        c.producer_backoff.short_cpu_relax_end = 4;
        c.producer_backoff.long_cpu_relax_end  = 8;
        c.producer_backoff.yield_end           = 16;
        c.producer_backoff.short_sleep_end     = 32;
        c.producer_backoff.long_sleep_ns       = 100000;

        c.misc.producer_timestamp = false;
    }
    //--------------------------------------------------------------------------
    void set_cfg (const cfg& c)
    {
        config = c;
        m_writer.prints_severity  = config.display.show_severity;
        m_writer.prints_timestamp = config.display.show_timestamp;
        m_wait.cfg                = c.consumer_backoff;
        /* corrections */
        if (config.queue.can_use_heap_q) {
            config.queue.bounded_q_blocking_sev = sev::off;
        }
    }
    //--------------------------------------------------------------------------
    bool validate_cfg (const cfg& c)
    {
        uword bsz = c.queue.bounded_q_block_size;
        uword esz = c.queue.bounded_q_entry_size;
        if ((!bsz || !esz) && !c.queue.can_use_heap_q) {
            std::cerr << "[logger] alloc cfg values can't be 0\n";
            assert (false && "alloc cfg invalid");
            return false;
        }
        if (bsz && esz) {
            if (esz < queue::min_entry_bytes) {
                std::cerr << "[logger] entry size too small, minimum size is: "
                          << queue::min_entry_bytes << "\n";
                return false;
            }
            else if (bsz < esz) {
                std::cerr << "[logger] entry size bigger than the block size\n";
                return false;
            }
            if ((bsz / esz) <= queue::min_entries) {
                std::cerr << "[logger] block size too small. requires "
                          << bsz / esz
                          << " bytes for the current entry size\n";
                return false;
            }
        }
        uword entries = (bsz && esz) ? (bsz / esz) :  0;
        if (!queue::validate_size_constraints(
            bsz, entries, c.queue.can_use_heap_q
            )) {
            std::cerr << "[logger] invalid queue configuration\n";
            return false;
        }
        if (c.file.rotation.file_count != 0 && c.file.aprox_size == 0) {
            std::cerr <<
                    "[logger] won't be able to rotate infinite size files\n";
            assert (false && "won't be able to rotate infinite size files");
            return false;
        }
        if (c.file.rotation.file_count == 1) {
            std::cerr << "[logger] won't be able to rotate a single file\n";
            assert (false && "won't be able to rotate a single file");
            return false;
        }
        if (c.file.out_folder.size() == 0) {
            std::cerr << "[logger] no output folder\n";
            assert (false && "log folder can't be empty");
            return false;
        }
        if (!m_files_register.can_write_in_folder (c.file.out_folder)) {
            return false;
        }
        return true;
    }
    //--------------------------------------------------------------------------
    void thread()
    {
        while (m_status.load (mo_acquire) != initialized) {                     // I guess that all Kernels do this for me when launching a thread, just being on the safe side in case is not true
            th::this_thread::yield();
        }
        idle_rotate_if();
        m_status.store (running, mo_relaxed);
        uword alloc_fault = m_alloc_fault.load (mo_relaxed);
        u64 next_flush = get_ns_timestamp() + (1 * 1000 * 1000 * 1000);
        u64 sev_check  = next_flush;
        change_current_filename();
        severity_check();

        while (true) {
            auto res = m_fifo.sc_pop_prepare();
            if (res.get_mem()) {
                if (file_error_avoidance()) { /*will print errors on stdout-stderr*/
                    non_idle_slice_and_rotate_if();
                }
                m_wait.reset();
                m_writer.decode_and_write (m_out, res.get_mem());
                m_fifo.pop_commit (res);
            }
            else {
                if (m_status.load (mo_relaxed) != running) {
                    break;
                }
                if (m_wait.next_wait_is_long_sleep()) {
                    idle_rotate_if();
                    auto now = get_ns_timestamp();
                    if (timestamp_is_expired (now, next_flush)) {
                        next_flush = now + (1 * 1000 * 1000 * 1000);
                        m_out.flush();
                    }
                    if (timestamp_is_expired (now, sev_check)) {
                        sev_check = now + (1 * 1000 * 1000 * 1000);
                        severity_check();
                    }
                }
                m_wait.wait();
            }
            uword allocf_now = m_alloc_fault.load (mo_relaxed);
            if (alloc_fault != allocf_now) {
                write_alloc_fault (allocf_now - alloc_fault);
                alloc_fault = allocf_now;
            }
        }
        idle_rotate_if();
        m_out.file_close();
        m_status.store (thread_stopped, mo_relaxed);
    }
    //--------------------------------------------------------------------------
    const char* change_current_filename()
    {
        using namespace ch;
        u64 cpu         = get_ns_timestamp();
        u64 calendar_us = duration_cast<microseconds>(
                        system_clock::now().time_since_epoch()
                        ).count();
        return m_files_register.change_current_filename (cpu, calendar_us);
    }
    //--------------------------------------------------------------------------
    void write_alloc_fault (uword count)
    {
        char str[96];
        mem_printf(
                str,
                sizeof str,
                "[%020llu] [logger_err] %u alloc faults detected",
                get_ns_timestamp(),
                count
                );
        m_out.raw_write (sev::error, str);
    }
    //--------------------------------------------------------------------------
    bool slices_files() const
    {
        return (config.file.aprox_size != 0);
    }
    //--------------------------------------------------------------------------
    bool has_to_slice_now()
    {
        return slices_files()
            && (m_out.file_bytes_written() >= config.file.aprox_size);
    }
    //--------------------------------------------------------------------------
    bool rotates() const
    {
        return (slices_files() && m_files_register.rotates());
    }
    //--------------------------------------------------------------------------
    bool reopen_file()
    {
        m_out.file_close();
        return m_out.file_open (m_files_register.current_filename());
    }
#define FILE_ERROR_AVOIDANCE_FMT_STR \
    "[%020llu] [logger_err] fatal filesystem error. entering error avoidance"
#define FILE_ERROR_AVOIDANCE_STR_MAX (sizeof FILE_ERROR_AVOIDANCE_FMT_STR + 20)
    //--------------------------------------------------------------------------
    bool file_error_avoidance()
    {
        bool success = m_out.file_is_open() && m_out.file_no_error();
        if (success) {
            m_on_error_avoidance = false;
            return true;
        }
        uword count = m_on_error_avoidance ? 16 : 1;
        for (uword i = 0; i < count; ++i) {
            success = reopen_file();
            if (success) {
                m_on_error_avoidance = false;
                return true;
            }
            if (m_on_error_avoidance) {
                th::this_thread::sleep_for (ch::milliseconds (100));
            }
        }
        if (m_on_error_avoidance == false) {
            char str[FILE_ERROR_AVOIDANCE_STR_MAX];
            mem_printf(
                str,
                sizeof str,
                FILE_ERROR_AVOIDANCE_FMT_STR,
                get_ns_timestamp()
                );
            m_out.raw_write (sev::error, str);
            m_on_error_avoidance = true;
        }
        if (!config.file.erase_and_retry_on_fatal_errors) {
            return false;
        }
        m_out.file_close();
        if (rotates()) {
            auto keep_count = config.file.rotation.file_count +
                config.file.rotation.delayed_file_count;
            while (keep_count-- != 0) {
                m_files_register.rotation_list_keep_newer (keep_count);
                success = m_out.file_open (change_current_filename());
                if (success) {
                    m_files_register.push_current_filename_to_rotation_list();
                    break;
                }
            }
        }
        else if (slices_files()) {
            success = non_idle_slice_and_rotate_if (true);
        }
        else {
            std::remove (m_files_register.current_filename());
            success = m_out.file_open (change_current_filename());
        }
        m_on_error_avoidance = !success;
        return success;
    }
    //--------------------------------------------------------------------------
    bool non_idle_slice_and_rotate_if(bool force = false)
    {
        bool success = true;
        if (force || has_to_slice_now()) {
            m_out.file_close();
            success = m_out.file_open (change_current_filename());
            if (success && rotates()) {
                m_files_register.rotation_list_keep_newer(
                    config.file.rotation.file_count +
                    config.file.rotation.delayed_file_count - 1
                    );
                m_files_register.push_current_filename_to_rotation_list();
            }
        }
        return success;
    }
    //--------------------------------------------------------------------------
    void idle_rotate_if()
    {
        if (rotates()) {
            m_files_register.rotation_list_keep_newer(
                config.file.rotation.file_count
                );
        }
    }
    //--------------------------------------------------------------------------
    void severity_check()
    {
        sev::severity err_prev, out_prev, file_prev, err, out, file;
        err  = err_prev  = m_out.stderr_sev();
        out  = out_prev  = m_out.stdout_sev();
        file = file_prev = m_out.file_sev();

        sev_read (file, config.sev.file_sev_fd);
        sev_read (err, config.sev.stderr_sev_fd);
        sev_read (out, config.sev.stdout_sev_fd);

        bool change = false;

        if (err_prev != err || out_prev != out) {
            if (out < err || out == sev::off) {
                m_out.set_console_severity (err, out);
                change = true;
            }
        }
        if (file_prev != file) {
            m_out.set_file_severity (file);
            change = true;
        }
        if (change && m_sev_evt) {
            m_sev_evt();
        }
    }
    //--------------------------------------------------------------------------
    void sev_read (sev::severity& s, const std::string& fd)
    {
        if (fd.size()) {
            int rs = sev_fd_check (fd.c_str());
            if (rs >= sev::debug && rs < sev::invalid) {
                s = (sev::severity) rs;
            }
        }
    }
    //--------------------------------------------------------------------------
    int sev_fd_check (const char* fd_path)
    {
        assert (fd_path);
        int v = -1;
        std::ifstream fd (fd_path);
        if (fd.is_open()) {
            fd >> v;
            return v;
        }
        return -1;
    }
    //--------------------------------------------------------------------------
    enum status {
        constructed,
        initializing,
        initialized,
        running,
        terminating,
        thread_stopped,
    };
    //--------------------------------------------------------------------------
    output              m_out;
    log_writer          m_writer;
    sev_update_evt      m_sev_evt;
    log_file_register   m_files_register;
    th::thread          m_log_thread;
    at::atomic<uword>   m_status;
    queue               m_fifo;
    sleep_queue_backoff m_wait;
    atomic_uword        m_alloc_fault;
    bool                m_on_error_avoidance;
 };
//------------------------------------------------------------------------------
} //namespaces

#endif /* MAL_LOG_BACKEND_DEF_HPP_ */
