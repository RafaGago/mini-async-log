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

#ifndef MAL_LOG_CALENDAR_TO_STR_HPP_
#define MAL_LOG_CALENDAR_TO_STR_HPP_

#include <cstdio>
#include <cassert>

#if defined (MAL_WINDOWS)
    #include <time.h>
#elif defined (MAL_UNIX_LIKE)
    #include <sys/time.h>
#else
    #error "unknown platform"
#endif

#include <mal_log/util/mem_printf.hpp>
#include <mal_log/util/system.hpp>
#include <mal_log/util/integer.hpp>

namespace mal {
//------------------------------------------------------------------------------
class calendar_str
{
public:
    static const uword str_size =                                               //non-null terminated
            4 + 1 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1 + 6;
    //      Y   -   M   -   D   _   H   :  min  :   s   .   us
    static const uword c_str_size = str_size + 1;

    static int write (char* dst, uword dst_capacity, u64 epoch_us)              //writes the non-null terminated str in dst
    {
        using namespace std;

        if (dst_capacity < c_str_size) { return -1; }

#if defined (MAL_32)
        const char* fmt = "%04d-%02d-%02d_%02d-%02d-%02d.%06d";
#elif defined (MAL_64)
        const char* fmt = "%04d-%02d-%02d_%02d-%02d-%02d.%06d";
#else
    #error "fix util/system.hpp for your platform (if possible)"
#endif

        time_t t = (time_t) (epoch_us / 1000000);
        tm cal;
#if defined (MAL_WINDOWS)
        localtime_s (&cal, &t);                                                  //localtime is thread safe in win
#elif defined (MAL_UNIX_LIKE)
        localtime_r (&t, &cal);
#else
    #error "implement this"
#endif
        auto micros = (int) (epoch_us - ((u64) t) * 1000000);
        auto res    = mem_printf(
                         dst,
                         dst_capacity,
                         fmt,
                         cal.tm_year + 1900,
                         cal.tm_mon + 1,
                         cal.tm_mday,
                         cal.tm_hour,
                         cal.tm_min,
                         cal.tm_sec,
                         micros
                         );
        assert (res == (decltype (res)) str_size);
        return res;
    }

}; //class calendar_to_str
//------------------------------------------------------------------------------

} //namespaces

#endif /* MAL_LOG_CALENDAR_TO_STR_HPP_ */
