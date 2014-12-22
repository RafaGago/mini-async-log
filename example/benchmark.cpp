/*
 * benchmark.cpp
 *
 *  Created on: Nov 24, 2014
 *      Author: rafgag
 */

#include <type_traits>

#include <mal_log/frontend_def.hpp>                                                //compiled in place, but it could be in a separate library
#include <mal_log/mal_log.hpp>

#include <mal_log/util/atomic.hpp>
#include <mal_log/util/thread.hpp>
#include <mal_log/util/chrono.hpp>
#include <mal_log/util/on_stack_dynamic.hpp>

#include <cstdio>
#include <memory>

#include <glog/logging.h>

#include <spdlog/spdlog.h>

#define TEST_LITERAL "message saying that something happened and an integer: "
#define OUT_FOLDER   "./log_out"
static const unsigned file_size_bytes = 50 * 1024 * 1024;

class mal_tester;
class spd_log_tester;

//------------------------------------------------------------------------------
template <class derived>
class perftest
{
public:
    //--------------------------------------------------------------------------
    bool run (mal::uword msgs, mal::uword thread_count)
    {
        using namespace mal;
        m_cummulative_enqueue_ns = 0;
        m_total_ns               = 0;
        m_alloc_faults           = 0;
        m_data_visible           = 0;

        static_cast<derived&> (*this).create();
        if (!static_cast<derived&> (*this).configure())
        {
            static_cast<derived&> (*this).destroy();
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

        static_cast<derived&> (*this).wait_until_work_completion();

        auto total_ns = (ch::duration_cast<ch::nanoseconds>(
                ch::steady_clock::now() - init
                ).count());

        static_cast<derived&> (*this).destroy();

        print_results (msgs, thread_count, total_ns, join_ns);

        return true;
    }
    //--------------------------------------------------------------------------
protected:
    //--------------------------------------------------------------------------
    void add_alloc_fault()
    {
        m_alloc_faults.fetch_add (1, mal::mo_relaxed);
    }
    //--------------------------------------------------------------------------
private:
    //--------------------------------------------------------------------------
    void thread (mal::uword msg_count)
    {
        using namespace mal;
        auto init = ch::steady_clock::now();

        static_cast<derived&> (*this).thread (msg_count);

        auto elapsed = ch::duration_cast<ch::nanoseconds>(
                ch::steady_clock::now() - init
                ).count();

        m_cummulative_enqueue_ns.fetch_add (elapsed, mal::mo_relaxed);
        m_data_visible.fetch_add (1, mal::mo_release);
    }
    //--------------------------------------------------------------------------
    template <class derived_>
    typename std::enable_if<!std::is_same<derived_, mal_tester>::value>::type
    print_alloc_faults()
    {

    }
    //--------------------------------------------------------------------------
    template <class derived_>
    typename std::enable_if<std::is_same<derived_, mal_tester>::value>::type
    print_alloc_faults()
    {
        std::printf(
            "%u fixed size queue alloc faults\n",
            m_alloc_faults.load (mal::mo_relaxed)
            );
    }
    //--------------------------------------------------------------------------
    template <class derived_>
    typename std::enable_if<
                std::is_same<derived_, spd_log_tester>::value
                >::type
    print_disk_times (mal::uword msgs, mal::u64 total_ns)
    {

    }
    //--------------------------------------------------------------------------
    template <class derived_>
    typename std::enable_if<
                !std::is_same<derived_, spd_log_tester>::value
                >::type
    print_disk_times (mal::uword msgs, mal::u64 total_ns)
    {
        double sec_ns  = 1000000000.;
        std::printf(
            "%f seconds spent writting to disk. msg/sec: %f \n",                //this is just reliable in ufo and
            ((double) total_ns) / sec_ns,
            ((double) msgs / (double) total_ns) * sec_ns
            );
    }
    //--------------------------------------------------------------------------
    void print_results(
            mal::uword msgs,
            mal::uword thread_count,
            mal::u64   total_ns,
            mal::u64   join_ns
            )
    {
        using namespace mal;
        while (m_data_visible.load (mal::mo_acquire) != thread_count)
        {
            th::this_thread::yield();
        }

        double sec_ns  = 1000000000.;
        double cumm_ns = (double)
                (m_cummulative_enqueue_ns.load (mal::mo_relaxed));

        std::printf(
            "%s using %u threads for a total of %u msgs\n",
            static_cast<derived&> (*this).get_name(),
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
    mal::u64                    m_total_ns;

    char pad1[256];

    mal::at::atomic<mal::u64>   m_cummulative_enqueue_ns;
    mal::at::atomic<mal::uword> m_data_visible;

    char pad2[256];

    mal::at::atomic<mal::uword> m_alloc_faults;
};
//------------------------------------------------------------------------------
class mal_tester : public perftest<mal_tester>
{
public:
    //--------------------------------------------------------------------------
    void set_params (mal::uword total_bsz, mal::uword entry_size, bool heap)
    {
        m_total_bsz  = total_bsz;
        m_entry_size = entry_size;
        m_heap       = heap;
    }
    //--------------------------------------------------------------------------
private:
    friend class perftest<mal_tester>;
    //--------------------------------------------------------------------------
    void create()  { m_fe.construct(); }
    //--------------------------------------------------------------------------
    void destroy() {  m_fe.destruct_if(); }
    //--------------------------------------------------------------------------
    bool configure()
    {
        using namespace mal;
        auto be_cfg                             = m_fe->get_backend_cfg();
        be_cfg.file.out_folder                  = OUT_FOLDER "/";               //this folder has to exist before running
        be_cfg.file.aprox_size                  = file_size_bytes;
        be_cfg.file.rotation.file_count         = 0;
        be_cfg.file.rotation.delayed_file_count = 0;                            //we let the logger to have an extra file when there is a lot of workload

        be_cfg.alloc.fixed_block_size           = m_total_bsz;
        be_cfg.alloc.fixed_entry_size           = m_entry_size;
        be_cfg.alloc.use_heap_if_required       = m_heap;

        m_fe->producer_timestamp (false);                                       //timestamping on producers slows the whole thing down 2.5, 3x, so we timestamp in the file worker for now. todo: for fairness the other libraries should have it disabled too

        return m_fe->init_backend (be_cfg) == frontend::init_ok;
    }
    //--------------------------------------------------------------------------
    void thread (mal::uword msg_count)
    {
        for (mal::u64 i = 0; i < msg_count; ++i)
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
    void wait_until_work_completion() { m_fe->on_termination(); }
    //--------------------------------------------------------------------------
    const char* get_name() { return "ufo log"; }
    //--------------------------------------------------------------------------
    mal::on_stack_dynamic<mal::frontend> m_fe;
    mal::uword m_total_bsz, m_entry_size, m_heap;
};
//------------------------------------------------------------------------------
class google_tester: public perftest<google_tester>
{
private:
    friend class perftest<google_tester>;
    //--------------------------------------------------------------------------
    void create()  {  }
    //--------------------------------------------------------------------------
    void destroy() {  }
    //--------------------------------------------------------------------------
    bool configure()
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
    void thread (mal::uword msg_count)
    {
        for (mal::u64 i = 0; i < msg_count; ++i)
        {
            LOG (ERROR) << TEST_LITERAL << i;
        }
    }
    //--------------------------------------------------------------------------
    void wait_until_work_completion()
    {
        google::FlushLogFiles (google::INFO);
    }
    //--------------------------------------------------------------------------
    const char* get_name() { return "glog"; }
    //--------------------------------------------------------------------------
};
//------------------------------------------------------------------------------
class spd_log_async_tester: public perftest<spd_log_async_tester>
{
private:
    friend class perftest<spd_log_async_tester>;
    //--------------------------------------------------------------------------
    void create()
    {
        spdlog::set_async_mode (250000);
        m_logger = spdlog::rotating_logger_mt(
                    "rotating_mt_async",
                    OUT_FOLDER "/" "spdlog_async",
                    file_size_bytes,
                    1000
                    );
        
    }
    //--------------------------------------------------------------------------
    void destroy()   { m_logger.reset(); }
    //--------------------------------------------------------------------------
    bool configure() { return true; }
    //--------------------------------------------------------------------------
    void thread (mal::uword msg_count)
    {
        for (mal::u64 i = 0; i < msg_count; ++i)
        {
            m_logger->info (log_fileline TEST_LITERAL, i);
        }
    }
    //--------------------------------------------------------------------------
    void wait_until_work_completion() {} //no way to know
    //--------------------------------------------------------------------------
    const char* get_name() { return "spdlog"; }
    //--------------------------------------------------------------------------
    std::shared_ptr<spdlog::logger> m_logger;
};
//------------------------------------------------------------------------------
class spd_log_sync_tester: public perftest<spd_log_sync_tester>
{
private:
    friend class perftest<spd_log_sync_tester>;
    //--------------------------------------------------------------------------
    void create()
    {
        spdlog::set_sync_mode ();
        m_logger = spdlog::rotating_logger_mt(
                    "rotating_mt_sync",
                    OUT_FOLDER "/" "spdlog_sync",
                    file_size_bytes,
                    1000
                    );
        
    }
    //--------------------------------------------------------------------------
    void destroy()   { m_logger.reset(); }
    //--------------------------------------------------------------------------
    bool configure() { return true; }
    //--------------------------------------------------------------------------
    void thread (mal::uword msg_count)
    {
        for (unsigned i = 0; i < msg_count; ++i)
        {
            m_logger->info (log_fileline TEST_LITERAL, i);
        }
    }
    //--------------------------------------------------------------------------
    void wait_until_work_completion() {}
    //--------------------------------------------------------------------------
    const char* get_name() { return "spdlog_sync"; }
    //--------------------------------------------------------------------------
    std::shared_ptr<spdlog::logger> m_logger;
};
//------------------------------------------------------------------------------
void mal_tests (mal::uword msgs)
{
    mal_tester mal_test;

//    std::printf ("pure heap----------------------------------------\n");
//
//    mal_test.set_params (0, 0, true);
//    mal_test.run (msgs, 1);
//
//    mal_test.set_params (0, 0, true);
//    mal_test.run (msgs, 2);
//
//    mal_test.set_params (0, 0, true);
//    mal_test.run (msgs, 4);
//
//    mal_test.set_params (0, 0, true);
//    mal_test.run (msgs, 8);
//
//    mal_test.set_params (0, 0, true);
//    mal_test.run (msgs, 16);
//
//    std::printf ("hybrid-------------------------------------------\n");
//
//    mal_test.set_params (1024 * 128, 32, true);
//    mal_test.run (msgs, 1);
//
//    mal_test.set_params (1024 * 128, 32, true);
//    mal_test.run (msgs, 2);
//
//    mal_test.set_params (1024 * 128, 32, true);
//    mal_test.run (msgs, 4);
//
//    mal_test.set_params (1024 * 128, 32, true);
//    mal_test.run (msgs, 8);

    mal_test.set_params (1024 * 128, 32, true);
    mal_test.run (msgs, 16);

//    std::printf ("no heap------------------------------------------\n");
//
//    mal_test.set_params (64 * 1024 * 1024, 32, false);
//    mal_test.run (msgs, 1);
//
//    mal_test.set_params (64 * 1024 * 1024, 32, false);
//    mal_test.run (msgs, 2);
//
//    mal_test.set_params (64 * 1024 * 1024, 32, false);
//    mal_test.run (msgs, 4);
//
//    mal_test.set_params (64 * 1024 * 1024, 32, false);
//    mal_test.run (msgs, 8);
//
//    mal_test.set_params (64 * 1024 * 1024, 32, false);
//    mal_test.run (msgs, 16);
}

//------------------------------------------------------------------------------
void do_a_pause()                                                               //time for the OS to finish some file io, otherwise some results were weird.
{
    using namespace mal;
    th::this_thread::sleep_for (ch::seconds (2));
}
//------------------------------------------------------------------------------
void google_tests (mal::uword msgs)
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
void spdlog_tests (mal::uword msgs)
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
    
    std::printf ("spdlog sync -------------------------------------------\n");
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
    const mal::uword msgs = 1600000;

    if (argc < 2)
    {
        std::printf ("no parameter specified (ufo, spdlog, glog)\n");
        return 1;
    }
    std::string choice = argv[1];

    if (choice.compare ("mal") == 0)
    {
        mal_tests (msgs);
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


