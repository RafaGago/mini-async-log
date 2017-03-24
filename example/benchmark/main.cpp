    /*
 * benchmark.cpp
 *
 *  Created on: Nov 24, 2014
 *      Author: rafgag
 */

#define NOMINMAX

#include <limits>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <math.h>
#include <type_traits>

#include <mal_log/frontend.hpp>                                                 //compiled in place, but it could be in a separate library
#include <mal_log/mal_log.hpp>

#include <mal_log/util/atomic.hpp>
#include <mal_log/util/thread.hpp>
#include <mal_log/util/chrono.hpp>
#include <mal_log/util/on_stack_dynamic.hpp>

#ifdef __linux__
    #include <pthread.h>
#endif

#include <glog/logging.h>
#include <spdlog/spdlog.h>

#define TEST_LITERAL "message saying that something happened and an integer: "
#define OUT_FOLDER   "./log_out"
static const unsigned file_size_bytes  = 50 * 1024 * 1024;

static const unsigned big_queue_bytes   =  1 * 1024 * 1024; /*1MB*/
static const unsigned queue_entry_size  =  32;
static const unsigned big_queue_entries =  big_queue_bytes / queue_entry_size;

class spd_log_async_perf_test;
//------------------------------------------------------------------------------
struct statistical_data {
    statistical_data()
    {
        mean = variance = stddev = min = max = 0.;
    }
    double mean;
    double variance;
    double stddev;
    double min;
    double max;
};
//------------------------------------------------------------------------------
struct timestamp_data {
    double start;
    double end;
};
//------------------------------------------------------------------------------
class statistical_accummulative_data {
public:
    //--------------------------------------------------------------------------
    statistical_accummulative_data()
    {
        m_mean_prev = 0.;
        m_n = 0;
    }
    //--------------------------------------------------------------------------
    void add_value (double x)
    {
        /*https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance*/
        ++m_n;
        double delta = x - sd.mean;
        sd.mean     += delta / (double) m_n;
        sd.variance += delta * (x - sd.mean);
        sd.min      = std::min (x, sd.min);
        sd.max      = std::max (x, sd.max);
    }
    //--------------------------------------------------------------------------
    void compute()
    {
        sd.variance = (m_n > 1) ? sd.variance / (double) (m_n - 1) : 0.;
        sd.stddev   = sqrt (sd.variance);
    }
    //--------------------------------------------------------------------------
    statistical_data sd;
private:
    double   m_mean_prev;
    unsigned m_n;
};
//------------------------------------------------------------------------------
template <class derived>
class perf_test
{
public:
    //--------------------------------------------------------------------------
    void print_throughput_header()
    {
        std::puts ("|threads|enqueue(s)|rate(Kmsgs/s)|total(s)|disk(Kmsgs/s)|all threads(s)|alloc faults|");
        std::puts ("|:-:|:-:|:-:|:-:|:-:|:-:|:-:|");
    }
    //--------------------------------------------------------------------------
    void print_latency_header()
    {
        std::puts ("|threads|clock|mean(us)|standard deviation|min(us)|max(us)|");
        std::puts ("|:-:|:-:|:-:|:-:|:-:|:-:|");
    }
    //--------------------------------------------------------------------------
    bool run_throughput (unsigned msgs, unsigned thread_count)
    {
        return run_throughput_impl (msgs, thread_count);
    }
    //--------------------------------------------------------------------------
    bool run_latency (unsigned msgs, unsigned thread_count)
    {
#ifdef __linux__
        if (!run_latency_impl(
                msgs,
                thread_count,
                &perf_test<derived>::latency_thread_thread_clock,
                "thread"
                )) {
            return false;
        }
#endif
        return run_latency_impl(
            msgs,
            thread_count,
            &perf_test<derived>::latency_thread_wall_clock,
            "wall"
            );
    }
    //--------------------------------------------------------------------------
private:

    template<class resultvector>
    struct thread_fn_signature{
        typedef typename
            resultvector::value_type (perf_test<derived>::*type) (unsigned);
    };
    //--------------------------------------------------------------------------
    template <class resultvector>
    bool execute_test(
        resultvector&                                    resvector,
        typename thread_fn_signature<resultvector>::type threadfn,
        unsigned                                         msgs,
        unsigned                                         thread_count,
        double&                                          total_sec
        )
    {
        m_alloc_faults.store (0, mal::mo_relaxed);
        typedef typename resultvector::value_type resulttype;
        using namespace mal;
        resvector.clear();
        resvector.insert (resvector.end(), thread_count, resulttype());
        static_cast<derived&> (*this).create();
        if (!static_cast<derived&> (*this).configure()) {
            static_cast<derived&> (*this).destroy();
            return false;
        }
        std::unique_ptr<th::thread[]> threads;
        if (thread_count > 1) {
            threads.reset (new th::thread [thread_count - 1]);
        }
        auto init = wall_clock_now_us();
        for (unsigned i = 0; i < (thread_count - 1); ++i) {
            threads[i] = th::thread ([=, &resvector]()
            {
                resvector[i] = (this->*threadfn) (msgs / thread_count);
            });
        }
        resvector[thread_count - 1] =
            (this->*threadfn)(msgs / thread_count);

        for (unsigned i = 0; i < (thread_count - 1); ++i) {
            threads[i].join();
        }
        static_cast<derived&> (*this).wait_until_work_completion();
        total_sec = (wall_clock_now_us() - init) / 1000000.;

        static_cast<derived&> (*this).destroy();
        return true;
    }
    //--------------------------------------------------------------------------
    bool run_throughput_impl (unsigned msgs, unsigned thread_count)
    {
        double total_sec;
        bool ret = execute_test(
            m_throughput_results,
            &perf_test<derived>::throughput_thread,
            msgs,
            thread_count,
            total_sec
            );
        if (!ret) {
            return ret;
        }
        double min_start      = std::numeric_limits<double>::max();
        double max_end        = 0.;
        double all_threads_sec = 0.;
        for (unsigned i = 0; i < thread_count; ++i) {
            double start = m_throughput_results[i].start;
            double end   = m_throughput_results[i].end;
            min_start = std::min (min_start, start);
            max_end   = std::max (max_end, end);
            all_threads_sec += (end - start);
        }
        double enqueue_sec      = (max_end - min_start) / 1000000.;
        all_threads_sec /= 1000000.;
        double throughput = (((double) msgs) / enqueue_sec) / 1000.;
        double disk       = (((double) msgs) / total_sec) / 1000.;
        if (std::is_same<derived, spd_log_async_perf_test>::value) {
            total_sec  = 0.;
            disk       = 0.;
        }
        std::printf(
            "|%u|%.3f|%.3f|%.3f|%.3f|%.3f|%u|\n",
            thread_count,
            enqueue_sec,
            throughput ,
            total_sec,
            disk,
            all_threads_sec,
            m_alloc_faults.load (mal::mo_relaxed)
            );
        std::fflush (stdout);
        return true;
    }
    //--------------------------------------------------------------------------
    timestamp_data throughput_thread (unsigned msg_count)
    {
        timestamp_data r;
        unsigned allocfaults = 0;
        r.start = wall_clock_now_us();
        for (mal::u64 i = 0; i < msg_count; ++i) {
            allocfaults += (unsigned) static_cast<derived&> (*this).log_one (i);
        }
        r.end = wall_clock_now_us();
        m_alloc_faults.fetch_add (allocfaults, mal::mo_relaxed);
        return r;
    }
    //--------------------------------------------------------------------------
    bool run_latency_impl(
        unsigned msgs,
        unsigned thread_count,
        statistical_data (perf_test<derived>::*threadfn) (unsigned),
        char const* clock_name
        )
    {
        double total_sec;
        bool ret = execute_test(
            m_latency_results,
            threadfn,
            msgs,
            thread_count,
            total_sec
            );
        if (!ret) {
            return ret;
        }
        statistical_data result;
        for (unsigned i = 0; i < thread_count; ++i) {
            statistical_data& c = m_latency_results[i];
            result.mean     += c.mean;
            result.variance += c.variance;
            result.min       = std::min (result.min, c.min);
            result.max       = std::max (result.max, c.max);
        }
        result.mean     /= (double) thread_count;
        result.variance /= (double) thread_count;
        result.stddev    = sqrt (result.variance);
        std::printf(
            "|%u|%s|%.3f|%.3f|%.3f|%.3f|\n",
            thread_count,
            clock_name,
            result.mean,
            result.stddev,
            result.min,
            result.max
            );
        std::fflush (stdout);
        return true;
    }
    //--------------------------------------------------------------------------
#ifdef __linux__
    struct thread_clock_id {
        thread_clock_id()
        {
            pthread_getcpuclockid (pthread_self(), &value);
        }
        clockid_t value;
    };
    //--------------------------------------------------------------------------
    double thread_clock_now_us (thread_clock_id& clock)
    {
        struct timespec ts;
        clock_gettime (clock.value, &ts);
        return (((double) ts.tv_sec) * 1000000) +
               (((double) ts.tv_nsec) * 0.001);
    }
    //--------------------------------------------------------------------------
    statistical_data latency_thread_thread_clock (unsigned msg_count)
    {
        thread_clock_id                clock;
        statistical_accummulative_data sad;
        for (mal::u64 i = 0; i < msg_count; ++i) {
            double start = thread_clock_now_us (clock);
            (void) static_cast<derived&> (*this).log_one (i);
            sad.add_value (thread_clock_now_us (clock) - start);
        }
        sad.compute();
        return sad.sd;
    }
#endif
    //--------------------------------------------------------------------------
    double wall_clock_now_us()
    {
        using namespace mal;
        auto v = ch::duration_cast<ch::nanoseconds>(
            ch::system_clock::now().time_since_epoch()
                ).count();
        return ((double) v) / 1000.;
    }
    //--------------------------------------------------------------------------
    statistical_data latency_thread_wall_clock (unsigned msg_count)
    {
        thread_clock_id                clock;
        statistical_accummulative_data sad;
        for (mal::u64 i = 0; i < msg_count; ++i) {
            double start = wall_clock_now_us();
            (void) static_cast<derived&> (*this).log_one (i);
            sad.add_value (wall_clock_now_us() - start);
        }
        sad.compute();
        return sad.sd;
    }
    //--------------------------------------------------------------------------
    mal::u64                      m_total_ns;
    mal::at::atomic<unsigned>     m_alloc_faults;
    std::vector<statistical_data> m_latency_results;
    std::vector<timestamp_data>   m_throughput_results;
};
//------------------------------------------------------------------------------
class mal_perf_test : public perf_test<mal_perf_test>
{
public:
    //--------------------------------------------------------------------------
    void set_params(
        unsigned total_bsz,
        unsigned entry_size,
        bool     heap,
        bool     block_on_full_queue = false
        )
    {
        m_total_bsz   = total_bsz;
        m_entry_size  = entry_size;
        m_heap        = heap;
        m_block_on_full_queue = block_on_full_queue;
    }
    //--------------------------------------------------------------------------
private:
    friend class perf_test<mal_perf_test>;
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
        m_fe->block_on_full_queue (m_block_on_full_queue);

        return m_fe->init_backend (be_cfg) == frontend::init_ok;
    }
    //--------------------------------------------------------------------------
    int log_one (unsigned i)
    {
        return (int) !log_error_i(
            m_fe.get(), log_fileline TEST_LITERAL "{}", i
            );
    }
    //--------------------------------------------------------------------------
    void wait_until_work_completion()
    {
        m_fe->on_termination();
    }
    //--------------------------------------------------------------------------
    const char* get_name() { return "mal log"; }
    //--------------------------------------------------------------------------
    mal::on_stack_dynamic<mal::frontend> m_fe;
    mal::th::mutex m_mutex;
    unsigned m_total_bsz, m_entry_size, m_heap;
    bool m_block_on_full_queue;
};
//------------------------------------------------------------------------------
class google_perf_test: public perf_test<google_perf_test>
{
private:
    friend class perf_test<google_perf_test>;
    //--------------------------------------------------------------------------
    void create()  {  }
    //--------------------------------------------------------------------------
    void destroy() {  }
    //--------------------------------------------------------------------------
    bool configure()
    {
        static bool configured = false;

        if (!configured) {
            FLAGS_log_dir         = OUT_FOLDER "/";
            FLAGS_logtostderr     = false;
            FLAGS_alsologtostderr = false;
            FLAGS_stderrthreshold = 3;
            FLAGS_max_log_size    = file_size_bytes ;
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
    int log_one (unsigned i)
    {
        LOG (ERROR) << log_fileline TEST_LITERAL << i;
        return 0;
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
class spd_log_async_perf_test: public perf_test<spd_log_async_perf_test>
{
private:
    friend class perf_test<spd_log_async_perf_test>;
    //--------------------------------------------------------------------------
    void create()
    {
        spdlog::set_async_mode (big_queue_entries);
        if (m_logger) {
            return;
        }
        m_logger = spdlog::rotating_logger_mt(
                    "rotating_mt_async",
                    OUT_FOLDER "/" "spdlog_async",
                    file_size_bytes,
                    1000
                    );

    }
    //--------------------------------------------------------------------------
    void destroy()   { /*m_logger.reset();*/ }
    //--------------------------------------------------------------------------
    bool configure() { return true; }
    //--------------------------------------------------------------------------
    int log_one (unsigned i)
    {
        m_logger->info (log_fileline TEST_LITERAL, i);
        return 0;
    }
    //--------------------------------------------------------------------------
    void wait_until_work_completion()
    {
        //is there a way to know when all logs have been written?
    }
    //--------------------------------------------------------------------------
    const char* get_name() { return "spdlog"; }
    //--------------------------------------------------------------------------
    std::shared_ptr<spdlog::logger> m_logger;
};
//------------------------------------------------------------------------------
class spd_log_sync_perf_test: public perf_test<spd_log_sync_perf_test>
{
private:
    friend class perf_test<spd_log_sync_perf_test>;
    //--------------------------------------------------------------------------
    void create()
    {
        spdlog::set_sync_mode ();
        if (m_logger) {
            return;
        }
        m_logger = spdlog::rotating_logger_mt(
                    "rotating_mt_sync",
                    OUT_FOLDER "/" "spdlog_sync",
                    file_size_bytes,
                    1000
                    );

    }
    //--------------------------------------------------------------------------
    void destroy()   { /*m_logger.reset();*/ }
    //--------------------------------------------------------------------------
    bool configure() { return true; }
    //--------------------------------------------------------------------------
    int log_one (unsigned i)
    {
        m_logger->info (log_fileline TEST_LITERAL, i);
        return 0;
    }
    //--------------------------------------------------------------------------
    void wait_until_work_completion() {}
    //--------------------------------------------------------------------------
    const char* get_name() { return "spdlog_sync"; }
    //--------------------------------------------------------------------------
    std::shared_ptr<spdlog::logger> m_logger;
};
//------------------------------------------------------------------------------
struct mal_params {
    unsigned queue_size;
    unsigned entry_size;
    bool     dynalloc;
    bool     blocking;
};
//------------------------------------------------------------------------------
void mal_tests (unsigned msgs)
{
    mal_perf_test mal_test;

    const mal_params params[] = {
        {0, 0, true, false},
        {big_queue_bytes / 16, queue_entry_size, true, false},
        {big_queue_bytes, queue_entry_size, false, true},
        {big_queue_bytes, queue_entry_size, false, false},
    };
    const char* headers[] = {
        "##pure heap (unbounded)",
        "##pure hybrid (unbounded)",
        "##no heap (blocking on full queue -> sync logger)",
        "##no heap (not blocking on full queue: lots of alloc faults)",
    };
    static const unsigned modecount = sizeof params / sizeof params[0];

    for (unsigned mode = 0; mode < modecount; ++mode) {
        std::printf ("%s\n\n", headers[mode]);
        mal_test.print_throughput_header();
        for (unsigned i = 0; i < 5; ++i) {
            mal_test.set_params(
                params[mode].queue_size,
                params[mode].entry_size,
                params[mode].dynalloc,
                params[mode].blocking
                );
            mal_test.run_throughput (msgs, 1 << i);
        }
        std::puts("");
        mal_test.print_latency_header();
        for (unsigned i = 0; i < 5; ++i) {
            mal_test.set_params(
                params[mode].queue_size,
                params[mode].entry_size,
                params[mode].dynalloc,
                params[mode].blocking
                );
            mal_test.run_latency (msgs, 1 << i);
        }
        std::puts("");
    }
}
//------------------------------------------------------------------------------
void long_pause()                                                               //time for the OS to finish some file io, otherwise some results were weird.
{
    using namespace mal;
    th::this_thread::sleep_for (ch::seconds (2));
}
//------------------------------------------------------------------------------
void google_tests (unsigned msgs)
{
    google_perf_test google_test;

    std::puts("##glog\n");
    google_test.print_throughput_header();
    for (unsigned i = 0; i < 5; ++i) {
        google_test.run_throughput (msgs, 1 << i);
        long_pause();
    }
    std::puts("");
    google_test.print_latency_header();
    for (unsigned i = 0; i < 5; ++i) {
        google_test.run_latency (msgs, 1 << i);
        long_pause();
    }
}
//------------------------------------------------------------------------------
void spdlog_tests (unsigned msgs)
{
    spd_log_async_perf_test spd_async_perf_test;                                //this one has no way to flush, so I place it the last to avoid affecting the next library measurements
    spd_log_sync_perf_test  spd_sync_perf_test;

    std::puts("##spdlog async\n");
    spd_async_perf_test.print_throughput_header();
    for (unsigned i = 0; i < 5; ++i) {
        spd_async_perf_test.run_throughput (msgs, 1 << i);
        long_pause();
    }
    std::puts("");
    spd_async_perf_test.print_latency_header();
    for (unsigned i = 0; i < 5; ++i) {
        spd_async_perf_test.run_latency (msgs, 1 << i);
        long_pause();
    }
    std::puts("\n##spdlog sync\n");
    spd_async_perf_test.print_throughput_header();
    for (unsigned i = 0; i < 5; ++i) {
        spd_async_perf_test.run_throughput (msgs, 1 << i);
        long_pause();
    }
    std::puts("");
    spd_async_perf_test.print_latency_header();
    for (unsigned i = 0; i < 5; ++i) {
        spd_async_perf_test.run_latency (msgs, 1 << i);
        long_pause();
    }
}
//------------------------------------------------------------------------------
int main (int argc, const char* argv[])
{
    const unsigned msgs = 2000000;

    if (argc < 2) {
        std::printf ("no parameter specified (mal, spdlog, glog)\n");
        return 1;
    }
    std::string choice = argv[1];

    if (choice.compare ("mal") == 0) {
        mal_tests (msgs);
    }
    else if (choice.compare ("glog") == 0) {
        google_tests (msgs);
    }
    else if (choice.compare ("spdlog") == 0) {
        spdlog_tests (msgs);
    }
    else {
        std::printf ("invalid choice\n");
        return 2;
    }
    return 0;
}
//------------------------------------------------------------------------------


