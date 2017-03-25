    /*
 * benchmark.cpp
 *
 *  Created on: Nov 24, 2014
 *      Author: rafgag
 */

#define NOMINMAX

#include <stdlib.h>

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
#include <mal_log/util/integer_bits.h>

#ifdef __linux__
    #include <pthread.h>
    #define HAS_THREAD_CLOCK 1
#else
    #define HAS_THREAD_CLOCK 0
#endif

#include <glog/logging.h>
#include <spdlog/spdlog.h>

#ifdef _WIN32
    #define DIR_SEP "\\"
    #define OUT_FOLDER ".\\mal_log_benchmarks"
    #define RM_OUT_FOLDER_ALL "del " OUT_FOLDER "\\*.*"
    #define MKDIR_OUT_FOLDER "md " OUT_FOLDER
#else
    #define DIR_SEP "/"
    #define OUT_FOLDER "./mal_log_benchmarks"
    #define RM_OUT_FOLDER_ALL "rm -f " OUT_FOLDER "/*"
    #define MKDIR_OUT_FOLDER "mkdir -p " OUT_FOLDER
#endif
#define TEST_LITERAL "message saying that something happened and an integer: "
static const unsigned file_size_bytes   = 50 * 1024 * 1024;
static const unsigned big_queue_bytes   = 1 * 1024 * 1024; /*1MB*/
static const unsigned queue_entry_size  = 32;
static const unsigned big_queue_entries = big_queue_bytes / queue_entry_size;

class spd_log_async_perf_test;
//------------------------------------------------------------------------------
class result_row {
public:
    char const* logger_name;
    unsigned    thread_count;
    result_row()
    {
        logger_name   = nullptr;
        thread_count  = 0;
        m_printconfig = 0;
    }
    void prints_logger_name (bool on)
    {
        SET_BIT_TO (m_printconfig, ilogger_name, on);
    }
    void prints_thread_count (bool on)
    {
        SET_BIT_TO (m_printconfig, ithread_count, on);
    }
    unsigned field_count()
    {
        return GET_BIT (m_printconfig, ilogger_name) +
               GET_BIT (m_printconfig, ithread_count);
    }
protected:
    void print_header()
    {
        std::printf ("|");
        if (field_count() == 0) {
            return;
        }
        for (unsigned i = 0; i < icount; ++i) {
            if (!GET_BIT (m_printconfig, i)) {
                continue;
            }
            switch(i) {
            case ilogger_name:
                std::printf ("logger|");
                break;
            case ithread_count:
                std::printf ("threads|");
                break;
            default:
                break;
            }
        }
    }
    void print_separator()
    {
        std::printf ("|");
        for (unsigned i = 0; i < field_count(); ++i) {
            std::printf (":-:|");
        }
    }
    void print_values()
    {
        std::printf ("|");
        if (field_count() == 0) {
            return;
        }
        for (unsigned i = 0; i < icount; ++i) {
            if (!GET_BIT (m_printconfig, i)) {
                continue;
            }
            switch(i) {
            case ilogger_name:
                std::printf ("%s|", logger_name);
                break;
            case ithread_count:
                std::printf ("%u|", thread_count);
                break;
            default:
                break;
            }
        }
    }
private:
    enum {
        ilogger_name,
        ithread_count,
        icount,
    };
    unsigned m_printconfig;
};
//------------------------------------------------------------------------------
struct throughput_data {
    throughput_data() {
        zero();
    }
    void zero() {
        enqueue_sec = msg_rate = total_sec = disk_rate = threads_sec = 0.;
        alloc_faults = 0.;
    }
    double enqueue_sec;
    double msg_rate;
    double total_sec;
    double disk_rate;
    double threads_sec;
    double alloc_faults;
};
//------------------------------------------------------------------------------
class throughput_row : public result_row, public throughput_data {
public:
    throughput_row (const char* logger_name = nullptr, unsigned threads = 0)
    {
        this->logger_name  = logger_name;
        this->thread_count = threads;
        zero();
    }
    void print_header()
    {
        result_row::print_header();
        std::puts(
            "enqueue(s)|rate(Kmsg/s)|total(s)|disk(Kmsg/s)|threads(s)|faults|"
            );
        result_row::print_separator();
        std::puts (":-:|:-:|:-:|:-:|:-:|:-:|");
    }
    void print_values()
    {
        result_row::print_values();
        std::printf(
            "%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|\n",
            enqueue_sec,
            msg_rate,
            total_sec,
            disk_rate,
            threads_sec,
            alloc_faults
            );
    }
};
//------------------------------------------------------------------------------
throughput_data average (std::vector<throughput_data>& in)
{
    throughput_data r;
    for (unsigned i = 0; i < in.size(); ++i) {
        throughput_data& c = in[i];
        r.enqueue_sec     += c.enqueue_sec;
        r.msg_rate        += c.msg_rate;
        r.total_sec       += c.total_sec;
        r.disk_rate       += c.disk_rate;
        r.threads_sec     += c.threads_sec;
        r.alloc_faults    += c.alloc_faults;
    }
    double count = (double) in.size();
    r.enqueue_sec  /= count;
    r.msg_rate     /= count;
    r.total_sec    /= count;
    r.disk_rate    /= count;
    r.threads_sec  /= count;
    r.alloc_faults /= count;
    return r;
}
//------------------------------------------------------------------------------
struct latency_data {
    latency_data()
    {
        zero();
    }
    void zero() {
        mean = variance = stddev = min = max = 0.;
    }
    double mean;
    double variance;
    double stddev;
    double min;
    double max;
};
//------------------------------------------------------------------------------
class latency_accumulator {
public:
    //--------------------------------------------------------------------------
    latency_accumulator()
    {
        m_mean_prev = 0.;
        m_n = 0;
    }
    //--------------------------------------------------------------------------
    void add_value (double x)
    {
        /*https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance*/
        ++m_n;
        double delta = x - ld.mean;
        ld.mean     += delta / (double) m_n;
        ld.variance += delta * (x - ld.mean);
        ld.min      = std::min (x, ld.min);
        ld.max      = std::max (x, ld.max);
    }
    //--------------------------------------------------------------------------
    void compute()
    {
        ld.variance = (m_n > 1) ? ld.variance / (double) (m_n - 1) : 0.;
        ld.stddev   = sqrt (ld.variance);
    }
    //--------------------------------------------------------------------------
    latency_data ld;
private:
    double   m_mean_prev;
    unsigned m_n;
};
//------------------------------------------------------------------------------
class latency_row : public result_row, public latency_data {
public:
    latency_row() {
        zero();
    }
    void print_header()
    {
        result_row::print_header();
        std::puts ("mean(us)|standard deviation|min(us)|max(us)|");
        result_row::print_separator();
        std::puts (":-:|:-:|:-:|:-:|");
    }
    void print_values()
    {
        result_row::print_values();
        std::printf("%.3f|%.3f|%.3f|%.3f|\n", mean, stddev, min, max);
    }
};
//------------------------------------------------------------------------------
latency_data average (std::vector<latency_data>& in)
{
    latency_data r;
    for (unsigned i = 0; i < in.size(); ++i) {
        latency_data& c = in[i];
        r.mean         += c.mean;
        r.variance     += c.variance;
        r.min           = std::min (r.min, c.min);
        r.max           = std::max (r.max, c.max);
    }
    r.mean     /= (double) in.size();
    r.variance /= (double) in.size();
    r.stddev    = sqrt (r.variance);
    return r;
}
//------------------------------------------------------------------------------
template <class derived>
class perf_test
{
public:
    //--------------------------------------------------------------------------
    bool run_throughput(
        throughput_data& r,
        unsigned         msgs,
        unsigned         thread_count
        )
    {
        return run_throughput_impl (r, msgs, thread_count);
    }
    //--------------------------------------------------------------------------
    bool run_latency(
        latency_data& r,
        unsigned      msgs,
        unsigned      thread_count,
        bool          wall
        )
    {
        if (wall) {
            return run_latency_impl(
                r,
                msgs,
                thread_count,
                &perf_test<derived>::latency_thread_wall_clock
                );
        }
        else {
#ifdef HAS_THREAD_CLOCK
            return run_latency_impl(
                r,
                msgs,
                thread_count,
                &perf_test<derived>::latency_thread_thread_clock
                );
#else
            return false;
#endif
        }
    }
    //--------------------------------------------------------------------------
private:
    //--------------------------------------------------------------------------
    struct time_interval {
        double start;
        double end;
    };
    //--------------------------------------------------------------------------
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
        bool ret = static_cast<derived&> (*this).wait_until_work_completion();
        total_sec = (wall_clock_now_us() - init) / 1000000.;
        if (!ret) {
            total_sec = 0.;
        }
        static_cast<derived&> (*this).destroy();
        return true;
    }
    //--------------------------------------------------------------------------
    bool run_throughput_impl(
        throughput_data& r, unsigned msgs, unsigned thread_count
        )
    {
        bool ret = execute_test(
            m_throughput_results,
            &perf_test<derived>::throughput_thread,
            msgs,
            thread_count,
            r.total_sec
            );
        if (!ret) {
            return ret;
        }
        r.zero();
        r.alloc_faults   = (double) m_alloc_faults.load (mal::mo_relaxed);
        double min_start = std::numeric_limits<double>::max();
        double max_end   = 0.;
        for (unsigned i = 0; i < thread_count; ++i) {
            double start = m_throughput_results[i].start;
            double end   = m_throughput_results[i].end;
            min_start = std::min (min_start, start);
            max_end   = std::max (max_end, end);
            r.threads_sec += (end - start);
        }
        r.enqueue_sec  = (max_end - min_start) / 1000000.;
        r.threads_sec /= 1000000.;
        r.disk_rate =
            (((double) msgs) - r.alloc_faults) / (r.total_sec * 1000.);
        r.msg_rate =
            (((double) msgs) - r.alloc_faults) / (r.enqueue_sec * 1000.);
        return true;
    }
    //--------------------------------------------------------------------------
    time_interval throughput_thread (unsigned msg_count)
    {
        time_interval r;
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
        latency_data&                      r,
        unsigned                           msgs,
        unsigned                           thread_count,
        latency_data (perf_test<derived>::*threadfn) (unsigned)
        )
    {
        double total_sec_dummy;
        bool ret = execute_test(
            m_latency_results,
            threadfn,
            msgs,
            thread_count,
            total_sec_dummy
            );
        if (!ret) {
            return ret;
        }
        r = average (m_latency_results);
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
    latency_data latency_thread_thread_clock (unsigned msg_count)
    {
        thread_clock_id     clock;
        latency_accumulator la;
        for (mal::u64 i = 0; i < msg_count; ++i) {
            double start = thread_clock_now_us (clock);
            (void) static_cast<derived&> (*this).log_one (i);
            la.add_value (thread_clock_now_us (clock) - start);
        }
        la.compute();
        return la.ld;
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
    latency_data latency_thread_wall_clock (unsigned msg_count)
    {
        thread_clock_id     clock;
        latency_accumulator la;
        for (mal::u64 i = 0; i < msg_count; ++i) {
            double start = wall_clock_now_us();
            (void) static_cast<derived&> (*this).log_one (i);
            la.add_value (wall_clock_now_us() - start);
        }
        la.compute();
        return la.ld;
    }
    //--------------------------------------------------------------------------
    mal::u64                   m_total_ns;
    mal::at::atomic<unsigned>  m_alloc_faults;
    std::vector<latency_data>  m_latency_results;
    std::vector<time_interval> m_throughput_results;
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
        auto be_cfg                     = m_fe->get_backend_cfg();
        be_cfg.file.out_folder          = OUT_FOLDER "/";               //this folder has to exist before running
        be_cfg.file.aprox_size          = file_size_bytes;
        be_cfg.file.rotation.file_count = 0;
        be_cfg.file.rotation.delayed_file_count = 0;                            //we let the logger to have an extra file when there is a lot of workload

        be_cfg.alloc.fixed_block_size     = m_total_bsz;
        be_cfg.alloc.fixed_entry_size     = m_entry_size;
        be_cfg.alloc.use_heap_if_required = m_heap;

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
    bool wait_until_work_completion()
    {
        m_fe->on_termination();
        return true;
    }
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
    bool wait_until_work_completion()
    {
        google::FlushLogFiles (google::INFO);
        return true;
    }
    //--------------------------------------------------------------------------
};
//------------------------------------------------------------------------------
class spd_log_perf_test: public perf_test<spd_log_perf_test>
{
public:
    //--------------------------------------------------------------------------
    void set_params(bool async)
    {
        m_async = async;
    }
private:
    friend class perf_test<spd_log_perf_test>;
    //--------------------------------------------------------------------------
    bool configure()
    {
        return true;
    }
    //--------------------------------------------------------------------------
    void create()
    {
        if (m_async) {
            spdlog::set_async_mode (big_queue_entries);
            m_logger = spdlog::rotating_logger_mt(
                "spdlog_logger",
                OUT_FOLDER DIR_SEP "spdlog_async",
                file_size_bytes,
                1000
                );
        }
        else {
            spdlog::set_sync_mode();
            m_logger = spdlog::rotating_logger_mt(
                "spdlog_logger",
                OUT_FOLDER DIR_SEP "spdlog_sync",
                file_size_bytes,
                1000
                );
        }
    }
    //--------------------------------------------------------------------------
    void destroy()
    {
        spdlog::drop ("spdlog_logger");
        m_logger.reset();
    }
    //--------------------------------------------------------------------------
    int log_one (unsigned i)
    {
        m_logger->info (log_fileline TEST_LITERAL, i);
        return 0;
    }
    //--------------------------------------------------------------------------
    bool wait_until_work_completion()
    {
        return false; /*No known way to flush the queue. No disk speed stats.*/
    }
    //--------------------------------------------------------------------------
    bool                            m_async;
    std::shared_ptr<spdlog::logger> m_logger;
};
//------------------------------------------------------------------------------
enum {
    log_mal_heap,
    log_mal_hybrid,
    log_spdlog_async,
    log_glog,
    log_mal_sync,
    log_spdlog_sync,
    log_mal_bounded,
    log_count,
};
//------------------------------------------------------------------------------
static char const* logger_names[] {
    "mal heap",
    "mal hybrid",
    "spdlog async",
    "glog",
    "mal sync",
    "spdlog sync",
    "mal bounded",
};
//------------------------------------------------------------------------------
template <class T>
void run_all_tests(
    T&               tester,
    throughput_data& rate,
    latency_data&    wall,
    latency_data&    cpu,
    unsigned         threads,
    unsigned         msgs
    )
{
    tester.run_throughput (rate, msgs, threads);
    tester.run_latency (wall, msgs, threads, true);
    tester.run_latency (cpu, msgs, threads, false);
}
//------------------------------------------------------------------------------
void run_tests(
    unsigned iterations, unsigned msgs, std::vector<bool> active_loggers
    )
{
    /*TODO: single thread table*/
    const unsigned threads_log2_max = 5;

    std::vector<throughput_data> rate[threads_log2_max][log_count];
    std::vector<latency_data>    wall[threads_log2_max][log_count];
    std::vector<latency_data>    cpu[threads_log2_max][log_count];

    for (unsigned t = 0; t < threads_log2_max; ++t) {
        for (unsigned i = 0; i < log_count; ++i) {
            rate[t][i].insert (rate[t][i].end(), iterations, throughput_data());
            wall[t][i].insert (wall[t][i].end(), iterations, latency_data());
            cpu[t][i].insert (cpu[t][i].end(), iterations, latency_data());
        }
    }
    spd_log_perf_test spdlog_test;
    google_perf_test  glog_test;
    mal_perf_test     mal_test;

    for (unsigned it = 0; it < iterations; ++it) {
        for (unsigned t = 0; t <  threads_log2_max; ++t) {
            unsigned thr = 1 << t;
            for (unsigned logger = 0; logger < log_count; ++logger) {
                if (!active_loggers[logger]) {
                    continue;
                }
                auto& c_rate = rate[t][logger][it];
                auto& c_wall = wall[t][logger][it];
                auto& c_cpu  = cpu[t][logger][it];
                printf(
                    "Run %u of %u. Processing %s with %u threads. %u msgs\n",
                    it + 1,
                    iterations,
                    logger_names[logger],
                    thr,
                    msgs
                    );
                switch (logger) {
                case log_mal_heap:
                    mal_test.set_params (0, 0, true, false);
                    run_all_tests (mal_test, c_rate, c_wall, c_cpu, thr, msgs);
                    break;
                case log_mal_hybrid:
                    mal_test.set_params(
                        big_queue_bytes / 16, queue_entry_size, true, false
                        );
                    run_all_tests (mal_test, c_rate, c_wall, c_cpu, thr, msgs);
                    break;
                case log_spdlog_async:
                    spdlog_test.set_params (true);
                    run_all_tests(
                        spdlog_test, c_rate, c_wall, c_cpu, thr, msgs
                        );
                    break;
                case log_glog:
                    run_all_tests (glog_test, c_rate, c_wall, c_cpu, thr, msgs);
                    break;
                case log_mal_sync:
                    mal_test.set_params(
                        big_queue_bytes, queue_entry_size, false, true
                        );
                    run_all_tests (mal_test, c_rate, c_wall, c_cpu, thr, msgs);
                    break;
                case log_spdlog_sync:
                    spdlog_test.set_params (false);
                    run_all_tests(
                        spdlog_test, c_rate, c_wall, c_cpu, thr, msgs
                        );
                    break;
                case log_mal_bounded:
                    mal_test.set_params(
                        big_queue_bytes, queue_entry_size, false, false
                        );
                    run_all_tests (mal_test, c_rate, c_wall, c_cpu, thr, msgs);
                    break;
                default:
                    break;
                }
                (void) system (RM_OUT_FOLDER_ALL); /*gcc crap: return deliberately ignored. I refuse to write bogus conditionals to shut up a compiler. The void cast should be enough.*/
            }
        }
    }
    throughput_row rate_row;
    latency_row    latency_row;

    rate_row.prints_logger_name (true);
    latency_row.prints_logger_name (true);

    puts ("Finished. Results:\n");

    for (unsigned t = 0; t < threads_log2_max; ++t) {
        unsigned threads = 1 << t;
        printf ("###threads: %u\n\n", threads);
        printf ("#### Throughput (threads=%u)\n\n", threads);
        rate_row.print_header();
        for (unsigned l = 0; l < log_count; ++l) {
            if (!active_loggers[l]) {
                    continue;
            }
            rate_row.logger_name = logger_names[l];
            ((throughput_data&) rate_row) = average (rate[t][l]);
            rate_row.print_values();
        }
        puts ("");
#ifdef HAS_THREAD_CLOCK
        printf ("#### Latency with thread clock (threads=%u)\n\n", threads);
        latency_row.print_header();
        for (unsigned l = 0; l < log_count; ++l) {
            if (!active_loggers[l]) {
               continue;
            }
            latency_row.logger_name = logger_names[l];
            ((latency_data&) latency_row) = average (cpu[t][l]);
            latency_row.print_values();
        }
        puts ("");
#endif
        printf ("#### Latency with wall clock (threads=%u)\n\n", threads);
        latency_row.print_header();
        for (unsigned l = 0; l < log_count; ++l) {
            if (!active_loggers[l]) {
               continue;
            }
            latency_row.logger_name = logger_names[l];
            ((latency_data&) latency_row) = average (wall[t][l]);
            latency_row.print_values();
        }
        puts ("");
    }
}
//------------------------------------------------------------------------------
int main (int argc, const char* argv[])
{
    const unsigned msgs = 2000000;

    if (system (MKDIR_OUT_FOLDER) == -1) {
        std::puts ("unable to create " OUT_FOLDER);
        return 3;
    }
    if (argc < 2) {
        std::printf ("no parameter specified (mal, spdlog, glog)\n");
        return 1;
    }
    std::string choice = argv[1];

    std::vector<bool> loggers;
    loggers.insert (loggers.end(), log_count, false);

    if (choice.compare ("full-test") == 0) {
        for (auto it = loggers.begin(); it < loggers.end(); ++it) {
            *it = true;
        }
        run_tests (20, msgs, loggers);
    }
    else if (choice.compare ("mal") == 0) {
        loggers[log_mal_heap] = true;
        loggers[log_mal_hybrid] = true;
        loggers[log_mal_sync] = true;
        loggers[log_mal_bounded] = true;
        run_tests (1, msgs, loggers);
    }
    else if (choice.compare ("glog") == 0) {
        loggers[log_glog] = true;
        run_tests (1, msgs, loggers);
    }
    else if (choice.compare ("spdlog") == 0) {
        loggers[log_spdlog_async] = true;
        loggers[log_spdlog_sync] = true;
        run_tests (1, msgs, loggers);
    }
    else {
        std::printf ("invalid choice\n");
        return 2;
    }
    return 0;
}
//------------------------------------------------------------------------------


