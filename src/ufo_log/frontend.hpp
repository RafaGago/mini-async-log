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

#ifndef UFO_LOG_LOG_FRONTEND_HPP_
#define UFO_LOG_LOG_FRONTEND_HPP_

#include <memory>
#include <ufo_log/message_encoder.hpp>

namespace ufo {

struct backend_cfg;

//------------------------------------------------------------------------------
class frontend
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
    backend_cfg get_backend_cfg();
    //--------------------------------------------------------------------------
    init_status init_backend (const backend_cfg& cfg);
    //--------------------------------------------------------------------------
    void set_severity (sev::severity s);
    //--------------------------------------------------------------------------
    sev::severity severity(); //todo: this will need to be renamed to lowest severity, and the backend will accept entries that are just destinated to the console
    //--------------------------------------------------------------------------
    bool set_console_severity(
            sev::severity stderr, sev::severity stdout = sev::off
            );
    //--------------------------------------------------------------------------
    proto::encoder get_encoder (uword required_bytes);
    //--------------------------------------------------------------------------
    void push_encoded (proto::encoder encoder);
    //--------------------------------------------------------------------------
    void on_termination();                                                      //you may want to call this from e.g. SIGTERM handlers, be aware that all the data generators should be stopped before.
    //--------------------------------------------------------------------------
private:
    class frontend_impl;
    std::unique_ptr<frontend_impl> m;

}; //class log_backed
//------------------------------------------------------------------------------
} //namespaces

#endif /* UFO_LOG_LOG_FRONTEND_HPP_ */
