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

#ifndef MAL_LOG_LOG_FRONTEND_HPP_
#define MAL_LOG_LOG_FRONTEND_HPP_

#include <mal_log/util/system.hpp>
#include <mal_log/serialization/exporter.hpp>
#include <mal_log/cfg.hpp>
#include <mal_log/util/thread.hpp>

namespace mal {

class sync_point;
//------------------------------------------------------------------------------
struct timestamp_data
{
    u64  base;
    bool producer_timestamps;
};
//------------------------------------------------------------------------------
class MAL_LIB_EXPORTED_CLASS frontend
{
public:
    //--------------------------------------------------------------------------
    enum init_status
    {
        init_ok,
        init_done_by_other,
        init_other_failed,
        init_tried_but_failed,
        init_was_terminated
    };
    //--------------------------------------------------------------------------
    frontend();
    //--------------------------------------------------------------------------
    ~frontend();
    //--------------------------------------------------------------------------
    bool is_constructed() const;
    //--------------------------------------------------------------------------
    cfg get_cfg();
    //--------------------------------------------------------------------------
    init_status init_backend (const cfg& cfg);
    //--------------------------------------------------------------------------
    sev::severity min_severity() const;
    //--------------------------------------------------------------------------
    bool can_log (sev::severity s) const;
    //--------------------------------------------------------------------------
    void set_file_severity (sev::severity s);
    //--------------------------------------------------------------------------
    bool set_console_severity(
            sev::severity std_err, sev::severity std_out = sev::off
            );
    //--------------------------------------------------------------------------
    ser::exporter get_encoder (uword required_bytes, sev::severity s);
    //--------------------------------------------------------------------------
    void async_push_encoded (ser::exporter& encoder);
    //--------------------------------------------------------------------------
    // this is an emergency call that blocks the caller until the entry is
    // dequeued by the file worker, it has more overhead and scales very poorly,
    // so if you are using this often you may need to switch to a traditional
    // synchronous-logger. returns false if interrupted/on termination.
    bool sync_push_encoded(
            ser::exporter& encoder,
            sync_point&    syncer
            );
    //--------------------------------------------------------------------------
    // you may want to call this from e.g. SIGTERM handlers, be aware that all
    // the data generators/producer should be stopped before to guarantee that
    // the queue can be left completely empty (no memory leaks).
    void on_termination();
    //--------------------------------------------------------------------------
    timestamp_data get_timestamp_data() const;
    //--------------------------------------------------------------------------
    u64 timestamp_base() const;
    //--------------------------------------------------------------------------
private:
    class frontend_impl;
    frontend_impl* m;

}; //class log_backed
//------------------------------------------------------------------------------
} //namespaces

#endif /* MAL_LOG_LOG_FRONTEND_HPP_ */
