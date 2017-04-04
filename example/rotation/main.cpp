/*
 * rotation.cpp
 *
 *  Created on: Nov 23, 2014
 *      Author: rafa
 */

#include <mal_log/extras/boost_filesystem_list_all_files.hpp>
#include <mal_log/mal_log.hpp>
//compiled in place, but it could be in a separate library
#include <mal_log/frontend.hpp>
#include <mal_log/util/chrono.hpp>
#include <cstdio>


//------------------------------------------------------------------------------
void rotation_test()
{
    using namespace mal;
    mal::frontend fe;

    //WARNING WARNING WARNING, files inside this folder will be deleted!
    const char* path = "./log_out/";

    auto mal_cfg                             = fe.get_cfg();
    mal_cfg.file.out_folder                  = path;
    mal_cfg.file.aprox_size                  = 2048 * 1024;
    mal_cfg.file.rotation.file_count         = 4;
    //we let the logger to have an extra file when there is a lot of workload
    mal_cfg.file.rotation.delayed_file_count = 1;
    mal_cfg.file.rotation.past_files         = extras::list_all_files (path);

    if (fe.init_backend (mal_cfg) != frontend::init_ok) { return; }

    const uword msg_count = 10000000;
    auto init = ch::steady_clock::now();

    for (unsigned i = 0; i < msg_count; ++i) {
        log_error_i (fe, "this is a very simple message {}", i);
    }
    auto reader_ns = ch::duration_cast<ch::nanoseconds>(
        ch::steady_clock::now() - init
        ).count();
    auto reader_msgs_s =
            ((double) msg_count / (double) reader_ns) * 1000000000.;

    fe.set_file_severity (sev::trace);

    log_trace_i(
        fe,
        "{} msgs enqueued in {} ns. {} msgs/sec ",
        msg_count,
        reader_ns,
        reader_msgs_s
        );
    // this gracefully shut downs the logger, waiting until the queues becomes
    // empty.
    fe.on_termination();

    auto writer_ns = ch::duration_cast<ch::nanoseconds>(
        ch::steady_clock::now() - init
        ).count();
    auto writer_msgs_s =
            ((double) msg_count / (double) writer_ns) * 1000000000.;

    std::printf(
        "%u msgs enqueued in %lld ns. %f msgs/sec\n",
        msg_count,
        reader_ns,
        reader_msgs_s
        );
    std::printf(
        "%u msgs dispatched in %lld ns. %f msgs/sec\n",
        msg_count,
        writer_ns,
        writer_msgs_s
        );
}
//------------------------------------------------------------------------------
int main (int argc, const char* argv[])
{
    rotation_test();
    return 0;
}
//------------------------------------------------------------------------------
