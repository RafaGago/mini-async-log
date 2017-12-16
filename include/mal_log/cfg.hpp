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

#ifndef MAL_LOG_LOG_BACKEND_CFG_HPP_
#define MAL_LOG_LOG_BACKEND_CFG_HPP_

#include <deque>
#include <string>
#include <mal_log/util/integer.hpp>
#include <mal_log/frontend_types.hpp>
#include <mal_log/util/queue_backoff_cfg.hpp>

namespace mal {

//------------------------------------------------------------------------------
typedef std::deque<std::string> past_executions_file_list;
//------------------------------------------------------------------------------
/* past_files: Past runs rotation related files. Odest at front.

      There is no portable way in the C++ standard either planned or developed
      that allows to list the files in a directory. So I can't add it without
      dependencies or platform specific code.

      So it's recommended that the user (or me in future versions) writes
      the platform specific code to iterate through the directory he can deliver
      the file list.

      This is trivial with e.g. boost::filesystem, but it can be the case that
      the user simply uses e.g. logrotate, I don't want to force anything now.

  file_count: simultaneous files to hold on disk. 0 = no rotation

  delayed_file_count: When this parameter is 0, the log files are rotated when
      starting a new file, you may not want to block the worker in a potentially
      time-consuming call on very high load, so setting this parameter to some
      value will allow the worker to skip rotation during high load peaks and
      to wait for the idle state before doing the operation.
*/
//------------------------------------------------------------------------------
struct rotation_cfg {
    past_executions_file_list past_files;
    uword                     file_count;
    uword                     delayed_file_count;
};
//------------------------------------------------------------------------------
/* out_folder: Existing out folder ended in slash/backslash depending on the
               platform. Can't be empty.
   aprox_size: Aproximate file slicing size. 0 = infinite: no file slicing

   erase_and_retry_on_fatal_errors: Gives permission to the logger to delete
              the current log file when an unrecoverable filesystem error has
              been found (e.g. disk full).
*/
//------------------------------------------------------------------------------
struct file_config {
    std::string   name_prefix;
    std::string   name_suffix;
    std::string   out_folder;
    uword         aprox_size;
    rotation_cfg  rotation;
    bool          erase_and_retry_on_fatal_errors;
};
//------------------------------------------------------------------------------
/* can_use_heap_q: the front end / cosumers are allowed to use the heap.
      This implies that when the log queue is full or when an entry bigger than
      the fixed-sized bucket has to be logged the logger can use the heap
      instead of reporting a failed operation.

   bounded_q_entry_size: max size for each log entry. (number of log entries =
      bounded_q_block_size / bounded_q_entry_size). If you are setting
      "use_heap_if_required" to "false" this value needs to have room for
      serializing the biggest log entry that you are planning to log.

   bounded_q_block_size: total queue byte size

   bounded_q_blocking_sev: when "can_use_heap_q" is "false", severities equal
      and above the value here will block the producer when the queue is full.
      Severities below won't block and will just report a failure.
      "mal::sev::off": blocking on full queue disabled.
      "mal::sev::debug": blocking on full queue fully enabled.
*/
//------------------------------------------------------------------------------
struct queue_config {
    bool          can_use_heap_q;
    uword         bounded_q_entry_size;
    uword         bounded_q_block_size;
    sev::severity bounded_q_blocking_sev;
};
//------------------------------------------------------------------------------
struct visualization_config {
    bool show_timestamp;
    bool show_severity;
};
//------------------------------------------------------------------------------
/* severity file paths. These files are tried to be read at runtime to
   dynamically change the logging severity for different sources. They only are
   read when the logger is idling.
*/
//------------------------------------------------------------------------------
struct severity_files {
    std::string file_sev_fd;
    std::string stdout_sev_fd;
    std::string stderr_sev_fd;
};
//------------------------------------------------------------------------------
/* producer_timestamp: gets the timestamp on the producer side: adds latency. */
//------------------------------------------------------------------------------
struct misc_settings {
    bool producer_timestamp;
};
//------------------------------------------------------------------------------
struct cfg {
    file_config          file;
    queue_config         queue;
    visualization_config display;
    severity_files       sev;
    queue_backoff_cfg    consumer_backoff; // read the code before tweaking
    queue_backoff_cfg    producer_backoff; // read the code before tweaking
    misc_settings        misc;
};
//------------------------------------------------------------------------------
} //namespaces
//------------------------------------------------------------------------------
#endif /* MAL_LOG_LOG_BACKEND_CFG_HPP_ */
