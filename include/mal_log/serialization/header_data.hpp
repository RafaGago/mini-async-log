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

#ifndef MAL_LOG_HEADER_DATA_HPP_
#define MAL_LOG_HEADER_DATA_HPP_

#include <mal_log/util/system.hpp>
#include <mal_log/util/integer.hpp>
#include <mal_log/frontend_types.hpp>

namespace mal {

class sync_point;

namespace ser {

//------------------------------------------------------------------------------
struct header_data
{
    u64           tstamp;                                                       //timestamps could be disabled on request, I don't know if us resolution would suffice.
    const char*   fmt;
    bool          has_tstamp;
    sev::severity severity;
    uword         arity;
    sync_point*   sync;
    uword         msg_size;
};
//------------------------------------------------------------------------------
inline header_data make_header_data(
                 sev::severity sev,
                 const char*   fmt,
                 uword         arity,
                 bool          has_tstamp = false,
                 u64           tstamp     = 0
                )
{
    header_data h;
    h.severity   = sev;
    h.fmt        = fmt;
    h.arity      = arity;
    h.has_tstamp = has_tstamp;
    h.tstamp     = tstamp;
    h.sync       = nullptr;
    return h;
}
//------------------------------------------------------------------------------

}} //namespaces

#endif /* MAL_LOG_HEADER_DATA_HPP_ */
