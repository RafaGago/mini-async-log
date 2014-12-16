/*
 * external_severity_change.cpp
 *
 *  Created on: Dec 16, 2014
 *      Author: rafgag
 */

#include <cassert>
#include <ufo_log/ufo_log.hpp>
#include <ufo_log/frontend_def.hpp> //UNCOMMENT IF YOU DON'T WANT TO COMPILE THE LIB SEPARATELY, COMMENT IF YOU DO
#include <ufo_log/util/stack_ostream.hpp>
#include <ufo_log/util/chrono.hpp>
#include <ufo_log/util/thread.hpp>

//------------------------------------------------------------------------------
inline ufo::frontend& get_ufo_logger_instance()
{
    static ufo::frontend fe;
    return fe;
}
//------------------------------------------------------------------------------
void run()
{
    using namespace ufo;
    ufo::frontend& fe                 = get_ufo_logger_instance();
    if (!fe.is_constructed())
    {
        return; //new failed in static initializator
    }
    auto be_cfg                       = fe.get_backend_cfg();
    be_cfg.file.name_prefix           = "test-data.";
    be_cfg.file.name_suffix           = ".log.txt";

#ifndef UFO_WINDOWS
    be_cfg.file.out_folder            = "./log_out/";                           //this folder has to exist before running
    be_cfg.sev.stderr_sev_fd          = "./stderr_sev";                         //the severity input file
#else
    be_cfg.file.out_folder            = ".\\log_out\\";                         //this folder has to exist before running
    be_cfg.sev.stderr_sev_fd          = ".\\stderr_sev";                        //the severity input file
#endif

    if (fe.init_backend (be_cfg) != frontend::init_ok) { return; }

    fe.set_file_severity (sev::off);
    fe.set_console_severity (sev::off);
    int i = 0;

    while (true)
    {
// by creating/modifying the file "./stderrsev" using values from 0 to 6 you
// can modify the console logged severity. In Linux you can enter
// "echo 0 > stderr_sev".

// These files are good candidates to be placed in memory mapped folders/drives.

        log_debug ("idx {}", i); ++i;
        log_trace ("idx {}", i); ++i;
        log_notice ("idx {}", i); ++i;
        log_warning ("idx {}", i); ++i;
        log_error ("idx {}", i); ++i;
        log_critical ("idx {}", i); ++i;
        th::this_thread::sleep_for (ch::milliseconds (200));
    }
}
//------------------------------------------------------------------------------
int main (int argc, const char* argv[])
{
    run();
    return 0;
}
//------------------------------------------------------------------------------


