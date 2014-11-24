/*
 * rotation.cpp
 *
 *  Created on: Nov 23, 2014
 *      Author: rafa
 */


#include <ufo_log/frontend_def.hpp>                                                //compiled in place, but it could be in a separate library
#include <ufo_log/ufo_log.hpp>
#include <ufo_log/util/chrono.hpp>
#include <cstdio>

//namespace ch = UFO_CHRONO_NAMESPACE;

//------------------------------------------------------------------------------
void rotation_test()
{
    using namespace ufo;
    ufo::frontend fe;
    auto be_cfg                             = fe.get_backend_cfg();
    be_cfg.file.out_folder                  = "./log_out/";                     //this folder has to exist before running
    be_cfg.file.aprox_size                  = 512 * 1024;
    be_cfg.file.rotation.file_count         = 4;
    be_cfg.file.rotation.delayed_file_count = 1;                                //we let the logger to have an extra file when there is a lot of workload

    if (fe.init_backend (be_cfg) != frontend::init_ok) { return; }

    const uword msg_count = 10000000;
    auto init = ch::steady_clock::now();

    for (unsigned i = 0; i < msg_count; ++i)
    {
        log_error_i (fe, "this is a very simple message {}", i);
    }
    auto reader_ns = ch::duration_cast<ch::nanoseconds>(
                        ch::steady_clock::now() - init
                        ).count();
    auto reader_msgs_s =
            ((double) msg_count / (double) reader_ns) * 1000000000.;

    fe.set_severity (sev::trace);

    log_trace_i(
        fe,
        "{} msgs enqueued in {} ns. {} msgs/sec ",
        msg_count,
        reader_ns,
        reader_msgs_s
        );
    fe.on_termination();                                                        //this gracefully shut downs the logger, wait until the queues become empty.

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
