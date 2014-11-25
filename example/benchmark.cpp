/*
 * benchmark.cpp
 *
 *  Created on: Nov 24, 2014
 *      Author: rafgag
 */

#include <type_traits>

#include <ufo_log/frontend_def.hpp>                                                //compiled in place, but it could be in a separate library
#include <ufo_log/ufo_log.hpp>

#include <ufo_log/util/atomic.hpp>
#include <ufo_log/util/thread.hpp>
#include <ufo_log/util/chrono.hpp>
#include <ufo_log/util/on_stack_dynamic.hpp>

#include <cstdio>
#include <memory>

#include <glog/logging.h>

#include <spdlog/spdlog.h>

//namespace ch = UFO_CHRONO_NAMESPACE;

namespace at = UFO_ATOMIC_NAMESPACE;
namespace th = UFO_THREAD_NAMESPACE;
namespace ch = UFO_CHRONO_NAMESPACE;

#define TEST_LITERAL "message saying that something happened and an integer: "
#define OUT_FOLDER   "./log_out"
static const unsigned file_size_bytes = 50 * 1024 * 1024;

class ufo_tester;
class spd_log_tester;

//------------------------------------------------------------------------------
template <class derived>
class perftest
{
public:
    //--------------------------------------------------------------------------
    bool run (ufo::uword msgs, ufo::uword thread_count)
    {
        m_cummulative_enqueue_ns   = 0;
        m_total_ns     = 0;
        m_alloc_faults = 0;
        m_data_visible = 0;

        static_cast<derived&> (*this).create_impl();
        if (!static_cast<derived&> (*this).configure_impl())
        {
            static_cast<derived&> (*this).destroy_impl();
            return false;
        }

        std::unique_ptr<th::thread[]> threads;
        if (thread_count > 1)
        {
            threads.reset (new th::thread [thread_count - 1]);
        }

        auto init = ch::steady_clock::now();

        for (unsigned i = 0; i < (thread_count - 1); ++i)
        {
            threads[i] = th::thread ([=]()
            {
                this->thread (msgs / thread_count);
            });
        }
        thread (msgs / thread_count);

        for (unsigned i = 0; i < (thread_count - 1); ++i)
        {
            threads[i].join();
        }

        auto join_ns = (ch::duration_cast<ch::nanoseconds>(
                ch::steady_clock::now() - init
                ).count());

        static_cast<derived&> (*this).flush_impl();

        auto total_ns = (ch::duration_cast<ch::nanoseconds>(
                ch::steady_clock::now() - init
                ).count());

        static_cast<derived&> (*this).destroy_impl();

        print_results (msgs, thread_count, total_ns, join_ns);

        return true;
    }
    //--------------------------------------------------------------------------
protected:
    //--------------------------------------------------------------------------
    void add_alloc_fault()
    {
        m_alloc_faults.fetch_add (1, ufo::mo_relaxed);
    }
    //--------------------------------------------------------------------------
private:
    //--------------------------------------------------------------------------
    void thread (ufo::uword msg_count)
    {
        auto init = ch::steady_clock::now();

        static_cast<derived&> (*this).thread_impl (msg_count);

        auto elapsed = ch::duration_cast<ch::nanoseconds>(
                ch::steady_clock::now() - init
                ).count();

        m_cummulative_enqueue_ns.fetch_add (elapsed, ufo::mo_relaxed);
        m_data_visible.fetch_add (1, ufo::mo_release);
    }
    //--------------------------------------------------------------------------
    template <class derived_>
    typename std::enable_if<!std::is_same<derived_, ufo_tester>::value>::type
    print_alloc_faults()
    {

    }
    //--------------------------------------------------------------------------
    template <class derived_>
    typename std::enable_if<std::is_same<derived_, ufo_tester>::value>::type
    print_alloc_faults()
    {
        std::printf(
            "%u fixed size queue alloc faults\n",
            m_alloc_faults.load (ufo::mo_relaxed)
            );
    }
    //--------------------------------------------------------------------------
    template <class derived_>
    typename std::enable_if<
                std::is_same<derived_, spd_log_tester>::value
                >::type
    print_disk_times (ufo::uword msgs, ufo::u64 total_ns)
    {

    }
    //--------------------------------------------------------------------------
    template <class derived_>
    typename std::enable_if<
                !std::is_same<derived_, spd_log_tester>::value
                >::type
    print_disk_times (ufo::uword msgs, ufo::u64 total_ns)
    {
        double sec_ns  = 1000000000.;
        std::printf(
            "%f seconds spent writting to disk. msg/sec: %f \n",              //this is just reliable in ufo and
            ((double) total_ns) / sec_ns,
            ((double) msgs / (double) total_ns) * sec_ns
            );
    }
    //--------------------------------------------------------------------------
    void print_results(
            ufo::uword msgs,
            ufo::uword thread_count,
            ufo::u64   total_ns,
            ufo::u64   join_ns
            )
    {
        while (m_data_visible.load (ufo::mo_acquire) != thread_count)
        {
            th::this_thread::yield();
        }

        double sec_ns  = 1000000000.;
        double cumm_ns = (double)
                (m_cummulative_enqueue_ns.load (ufo::mo_relaxed));

        std::printf(
            "%s using %u threads for a total of %u msgs\n",
            static_cast<derived&> (*this).get_name_impl(),
            thread_count,
            msgs
            );
        print_alloc_faults<derived>();
        print_disk_times<derived> (msgs, total_ns);

        std::printf(
            "avg thread duration: %f .cummulative seconds spent between"
            " all threads. %f\n",
            (cumm_ns / sec_ns) / (double) thread_count,
            cumm_ns / sec_ns
            );
        std::printf(
            "%f seconds spent before all threads joined. enqueued msg/sec: "
            "%f\n\n",
            ((double) join_ns) / sec_ns,
            ((double) msgs / (double) join_ns) * sec_ns
            );
        std::fflush (stdout);

    }
    //--------------------------------------------------------------------------
    ufo::u64                    m_total_ns;

    char pad1[256];

    ufo::at::atomic<ufo::u64>   m_cummulative_enqueue_ns;
    ufo::at::atomic<ufo::uword> m_data_visible;

    char pad2[256];

    ufo::at::atomic<ufo::uword> m_alloc_faults;
};
//------------------------------------------------------------------------------
class ufo_tester : public perftest<ufo_tester>
{
public:
    //--------------------------------------------------------------------------
    void set_params (ufo::uword entries, ufo::uword entry_size, bool heap)
    {
        m_entries    = entries;
        m_entry_size = entry_size;
        m_heap       = heap;
    }
    //--------------------------------------------------------------------------
private:
    friend class perftest<ufo_tester>;
    //--------------------------------------------------------------------------
    void create_impl()  { m_fe.construct(); }
    //--------------------------------------------------------------------------
    void destroy_impl() {  m_fe.destruct_if(); }
    //--------------------------------------------------------------------------
    bool configure_impl()
    {
        using namespace ufo;
        auto be_cfg                             = m_fe->get_backend_cfg();
        be_cfg.file.out_folder                  = OUT_FOLDER "/";               //this folder has to exist before running
        be_cfg.file.aprox_size                  = file_size_bytes;
        be_cfg.file.rotation.file_count         = 0;
        be_cfg.file.rotation.delayed_file_count = 0;                                //we let the logger to have an extra file when there is a lot of workload

        be_cfg.alloc.fixed_size_entry_count     = m_entries;
        be_cfg.alloc.fixed_size_entry_size      = m_entry_size;
        be_cfg.alloc.use_heap_if_required       = m_heap;

        return m_fe->init_backend (be_cfg) == frontend::init_ok;
    }
    //--------------------------------------------------------------------------
    void thread_impl (ufo::uword msg_count)
    {
        for (unsigned i = 0; i < msg_count; ++i)
        {
            bool res =
                log_error_i (m_fe.get(), log_fileline TEST_LITERAL "{}", i);
            if (!res)
            {
                add_alloc_fault();
            }
        }
    }
    //--------------------------------------------------------------------------
    void flush_impl()           { m_fe->on_termination(); }
    //--------------------------------------------------------------------------
    const char* get_name_impl() { return "ufo log"; }
    //--------------------------------------------------------------------------
    ufo::on_stack_dynamic<ufo::frontend> m_fe;
    ufo::uword m_entries, m_entry_size, m_heap;

};
//------------------------------------------------------------------------------
class google_tester: public perftest<google_tester>
{
private:
    friend class perftest<google_tester>;
    //--------------------------------------------------------------------------
    void create_impl()  {  }
    //--------------------------------------------------------------------------
    void destroy_impl() {  }
    //--------------------------------------------------------------------------
    bool configure_impl()
    {
        static bool configured = false;

        if (!configured)
        {
            FLAGS_log_dir         = OUT_FOLDER "/";
            FLAGS_logtostderr     = false;
            FLAGS_alsologtostderr = false;
            FLAGS_stderrthreshold = 3;
            FLAGS_max_log_size    = file_size_bytes / (1024 * 1024);
            google::SetLogDestination (google::INFO, OUT_FOLDER "/");
            google::SetLogDestination (google::WARNING, "");
            google::SetLogDestination (google::ERROR, "");
            google::SetLogDestination (google::FATAL, "");
            google::SetLogFilenameExtension ("");
            google::InitGoogleLogging ("");
            configured = true;
        }
        return true;
    }
    //--------------------------------------------------------------------------
    void thread_impl (ufo::uword msg_count)
    {
        for (unsigned i = 0; i < msg_count; ++i)
        {
            LOG(ERROR) << TEST_LITERAL << i;
        }
    }
    //--------------------------------------------------------------------------
    void flush_impl()           { google::FlushLogFiles (google::INFO); }
    //--------------------------------------------------------------------------
    const char* get_name_impl() { return "glog"; }
    //--------------------------------------------------------------------------
};
//------------------------------------------------------------------------------
class spd_log_async_tester: public perftest<spd_log_async_tester>
{
private:
    friend class perftest<spd_log_async_tester>;
    //--------------------------------------------------------------------------
    void create_impl()
    {
        //Fixed queue size of few thousands give best results
        
        m_logger = spdlog::rotating_logger_mt(
                    "rotating_mt_async",
                    OUT_FOLDER "/" "spdlog_async",
                    file_size_bytes,
                    1000
                    );
        
    }
    //--------------------------------------------------------------------------
    void destroy_impl()
    {
        m_logger.reset();
    }
    //--------------------------------------------------------------------------
    bool configure_impl()
    {
        return true;
    }
    //--------------------------------------------------------------------------
    void thread_impl (ufo::uword msg_count)
    {
        for (unsigned i = 0; i < msg_count; ++i)
        {
            m_logger->info (log_fileline TEST_LITERAL, i);
        }
    }
    //--------------------------------------------------------------------------
    void flush_impl()           { //flush is done automatically} 
    //--------------------------------------------------------------------------
    const char* get_name_impl() { return "spdlog"; }
    //--------------------------------------------------------------------------
    std::shared_ptr<spdlog::logger> m_logger;
};
//------------------------------------------------------------------------------
class spd_log_sync_tester: public perftest<spd_log_sync_tester>
{
private:
    friend class perftest<spd_log_sync_tester>;
    //--------------------------------------------------------------------------
    void create_impl()
    {
        m_logger = spdlog::rotating_logger_mt(
                    "rotating_mt_sync",
                    OUT_FOLDER "/" "spdlog_sync",
                    file_size_bytes,
                    1000
                    );
        
    }
    //--------------------------------------------------------------------------
    void destroy_impl()
    {
        m_logger.reset();
    }
    //--------------------------------------------------------------------------
    bool configure_impl()
    {
        return true;
    }
    //--------------------------------------------------------------------------
    void thread_impl (ufo::uword msg_count)
    {
        for (unsigned i = 0; i < msg_count; ++i)
        {
            m_logger->info (log_fileline TEST_LITERAL, i);
        }
    }
    //--------------------------------------------------------------------------
    void flush_impl()           { //flush is done automatically} 
    //--------------------------------------------------------------------------
    const char* get_name_impl() { return "spdlog_sync"; }
    //--------------------------------------------------------------------------
    std::shared_ptr<spdlog::logger> m_logger;
};
//------------------------------------------------------------------------------
void ufo_tests (ufo::uword msgs)
{
    ufo_tester ufo_test;

    std::printf ("no heap------------------------------------------\n");

    ufo_test.set_params (1024 * 1024, 16, false);
    ufo_test.run (msgs, 1);

    ufo_test.set_params (1024 * 1024, 16, false);
    ufo_test.run (msgs, 2);

    ufo_test.set_params (1024 * 1024, 16, false);
    ufo_test.run (msgs, 4);

    ufo_test.set_params (1024 * 1024, 16, false);
    ufo_test.run (msgs, 8);

    std::printf ("pure heap----------------------------------------\n");

    ufo_test.set_params (0, 0, true);
    ufo_test.run (msgs, 1);

    ufo_test.set_params (0, 0, true);
    ufo_test.run (msgs, 2);

    ufo_test.set_params (0, 0, true);
    ufo_test.run (msgs, 4);

    ufo_test.set_params (0, 0, true);
    ufo_test.run (msgs, 8);

    std::printf ("hybrid-------------------------------------------\n");

    ufo_test.set_params (1024 * 8, 16, true);
    ufo_test.run (msgs, 1);

    ufo_test.set_params (1024 * 8, 16, true);
    ufo_test.run (msgs, 2);

    ufo_test.set_params (1024 * 8, 16, true);
    ufo_test.run (msgs, 4);

    ufo_test.set_params (1024 * 8, 16, true);
    ufo_test.run (msgs, 8);
}
//------------------------------------------------------------------------------
void do_a_pause()                                                               //time for the OS to finish some file io, otherwise some results were weird.
{
    th::this_thread::sleep_for (ch::seconds (2));
}
//------------------------------------------------------------------------------
void google_tests (ufo::uword msgs)
{
    google_tester google_test;

    google_test.run (msgs, 1);
    do_a_pause();

    google_test.run (msgs, 2);
    do_a_pause();

    google_test.run (msgs, 4);
    do_a_pause();

    google_test.run (msgs, 8);
    do_a_pause();
}
//------------------------------------------------------------------------------
void spdlog_tests (ufo::uword msgs)
{
    std::printf ("spdlog async ------------------------------------------\n");
    spd_log_async_tester spd_async_tester;                                                  //this one has no way to flush, so I place it the last to avoid affecting the next library measurements

    spd_async_tester.run (msgs, 1);
    do_a_pause();

    spd_async_tester.run (msgs, 2);
    do_a_pause();

    spd_async_tester.run (msgs, 4);
    do_a_pause();

    spd_async_tester.run (msgs, 8);
    do_a_pause();
    
    std::printf ("spdlog sync ------------------------------------------\n");
    spd_log_sync_tester spd_sync_tester;

    spd_sync_tester.run (msgs, 1);
    do_a_pause();

    spd_sync_tester.run (msgs, 2);
    do_a_pause();

    spd_sync_tester.run (msgs, 4);
    do_a_pause();

    spd_sync_tester.run (msgs, 8);
    do_a_pause();

    spdlog::stop();
}
//------------------------------------------------------------------------------
int main (int argc, const char* argv[])
{
    const ufo::uword msgs = 1600000;

    if (argc < 2)
    {
        std::printf ("no parameter specified (ufo, spdlog, glog)\n");
        return 1;
    }

    std::string choice = argv[1];

    if (choice.compare ("ufo") == 0)
    {
        ufo_tests (msgs);
    }
    else if (choice.compare ("glog") == 0)
    {
        google_tests (msgs);
    }
    else if (choice.compare ("spdlog") == 0)
    {
        spdlog_tests (msgs);
    }
    else
    {
        std::printf ("invalid choice\n");
        return 2;
    }
    return 0;
}
//------------------------------------------------------------------------------


