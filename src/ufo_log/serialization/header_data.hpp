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

#ifndef UFO_LOG_HEADER_DATA_HPP_
#define UFO_LOG_HEADER_DATA_HPP_

#include <ufo_log/util/system.hpp>
#include <ufo_log/util/integer.hpp>
#include <ufo_log/frontend_types.hpp>

namespace ufo {

//------------------------------------------------------------------------------
struct header_data
{
    u64           tstamp;                                                       //timestamps could be disabled on request, I don't know if us resolution would suffice.
    const char*   fmt;
    bool          has_tstamp;
    sev::severity severity;
    uword         arity;
#ifndef UFO_COMPILE_TIME_FMT_CHECK
    uword         msg_bytes;
#endif
};
//------------------------------------------------------------------------------
#ifdef UFO_COMPILE_TIME_FMT_CHECK
//------------------------------------------------------------------------------
header_data make_header_data(
                 sev::severity sev,
                 const char*   fmt,
                 uword         arity,
                 bool          has_tstamp = false,
                 u64           tstamp     = 0,
                )
{
    header_data h;
    h.severity   = sev;
    h.fmt        = fmt;
    h.arity      = arity;
    h.has_tstamp = has_tstamp;
    h.tstamp     = tstamp;
    return h;
}
//------------------------------------------------------------------------------
#else //UFO_COMPILE_TIME_FMT_CHECK
//------------------------------------------------------------------------------
header_data make_header_data(
                 sev::severity sev,
                 const char*   fmt,
                 uword         arity,
                 uword         msg_bytes,
                 bool          has_tstamp = false,
                 u64           tstamp     = 0,
                )
{
    header_data h;
    h.severity   = sev;
    h.fmt        = fmt;
    h.arity      = arity;
    h.msg_bytes  = msg_bytes;
    h.has_tstamp = has_tstamp;
    h.tstamp     = tstamp;
    return h;
}
//------------------------------------------------------------------------------
#endif //UFO_COMPILE_TIME_FMT_CHECK
//------------------------------------------------------------------------------


} //namespaces

#endif /* UFO_LOG_HEADER_DATA_HPP_ */
