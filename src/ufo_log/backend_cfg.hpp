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

#ifndef UFO_LOG_LOG_BACKEND_CFG_HPP_
#define UFO_LOG_LOG_BACKEND_CFG_HPP_

#include <deque>
#include <string>
#include <ufo_log/util/integer.hpp>
#include <ufo_log/util/mpsc_hybrid_wait.hpp>

namespace ufo {

//------------------------------------------------------------------------------
typedef std::deque<std::string> past_executions_file_list;
//------------------------------------------------------------------------------
//
// There is no portable way in the C++ standard either planned or developed
// that allows to list the files in a directory. So I can't add it without
// dependencies or platform specific code.
//
// So it's recommended that the user (or me in future versions) writes
// the platform specific code to iterate through the directory he can deliver
// the file list.
//
// This is trivial with e.g. boost::filesystem, but it can be the case that
// the user simply uses e.g. logrotate, I don't want to force anything now.
//
//------------------------------------------------------------------------------
struct backend_rotation_cfg
{
    past_executions_file_list past_files;                                       //oldest at front
    uword                     file_count;                                       //0 = infinite/externally managed (e.g. logrotate)
    uword                     delayed_file_count;                               //when this parameter is 0, the log files are rotated when starting a new file, you may not want to block the worker in a potentially time-consuming call, so setting this parameter to some value will allow the worker to skip rotating during high load peaks.
};
//------------------------------------------------------------------------------
struct backend_file_config
{
    std::string          name_prefix;
    std::string          name_suffix;
    std::string          out_folder;                                            //Existing out folder ended in slash/backslash depending on the platform. Can't be zero sized
    uword                aprox_size;                                            //0 = infinite
    backend_rotation_cfg rotation;
};
//------------------------------------------------------------------------------
struct backend_log_entry_alloc_config                                           //the allocator for log entries, the backend still uses the heap for some std::strings when rotating files. I found excessive to use fixed size static strings at this point.
{
    bool  use_heap_if_required;
    uword fixed_block_size;
    uword fixed_entry_size;
};
//------------------------------------------------------------------------------
struct backend_visualization_config
{
    bool show_timestamp;
    bool show_severity;
};
//------------------------------------------------------------------------------
struct backend_cfg
{
    backend_file_config            file;
    backend_log_entry_alloc_config alloc;
    backend_visualization_config   display;
    mpsc_hybrid_wait_cfg           blocking;
}; //class log_backend_cfg
//------------------------------------------------------------------------------
} //namespaces
//------------------------------------------------------------------------------
#endif /* UFO_LOG_LOG_BACKEND_CFG_HPP_ */
