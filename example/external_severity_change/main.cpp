/*
 * external_severity_change.cpp
 *
 *  Created on: Dec 16, 2014
 *      Author: rafgag
 */

#include <cassert>
#include <mal_log/mal_log.hpp>
#include <mal_log/frontend.hpp> //UNCOMMENT IF YOU DON'T WANT TO COMPILE THE LIB SEPARATELY, COMMENT IF YOU DO
#include <mal_log/util/stack_ostream.hpp>
#include <mal_log/util/chrono.hpp>
#include <mal_log/util/thread.hpp>

//------------------------------------------------------------------------------
inline mal::frontend& get_mal_logger_instance()
{
    static mal::frontend fe;
    return fe;
}
//------------------------------------------------------------------------------
void run()
{
    using namespace mal;
    mal::frontend& fe                 = get_mal_logger_instance();
    if (!fe.is_constructed()) {
        return; //new failed in static initializator
    }
    auto mal_cfg             = fe.get_cfg();
    mal_cfg.file.name_prefix = "test-data.";
    mal_cfg.file.name_suffix = ".log.txt";

#ifndef MAL_WINDOWS
    //this folder has to exist before running
    mal_cfg.file.out_folder   = "./log_out/";
    //the severity input file
    mal_cfg.sev.stderr_sev_fd = "./stderr_sev";
#else
    //this folder has to exist before running
    mal_cfg.file.out_folder    = ".\\log_out\\";
    //the severity input file
    mal_cfg.sev.stderr_sev_fd  = ".\\stderr_sev";
#endif
    if (fe.init_backend (mal_cfg) != frontend::init_ok) { return; }

    fe.set_file_severity (sev::off);
    fe.set_console_severity (sev::off);
    int i = 0;

    while (true) {
// by creating/modifying the file "./stderrsev" using values from 0 to 6 you
// can modify the console logged severity. In Linux you can enter
// "echo 0 > stderr_sev".

// These files are good candidates to be placed on memory mapped folders/drives.
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


