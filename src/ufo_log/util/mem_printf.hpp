/*
The BSD 3-clause license

Copyright (c) 2013-2014 Diadrom AB. All rights reserved.

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

THIS SOFTWARE IS PROVIDED BY DIADROM AB "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
SHALL DIADROM AB OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of Diadrom AB.
--------------------------------------------------------------------------------
*/

#ifndef UFO_UFO_MEM_PRINTF_HPP_
#define UFO_UFO_MEM_PRINTF_HPP_

#include <cassert>
#include <stdarg.h>
#include <cstdio>
#include <ufo_log/util/system.hpp>

namespace ufo {

// This function just returns the bytes written excluding the null character,
// the value is positive when not truncated and negative when truncated.
// 
// The trailing null is always appended inside the passed buffer.
// 
// It is just used to standarize behavior and remove warnings. The Windows
// documentation is confusing too.

//------------------------------------------------------------------------------
inline int mem_printf (char* mem, int mem_size, const char* fmt, ...)
{
    assert (mem && fmt);
    if (mem && (mem_size > 0) && fmt && (fmt[0] != 0))
    {
        va_list args;
        va_start (args, fmt);
#ifdef UFO_WINDOWS
    #pragma warning(disable: 4996)
#endif
        int res = vsnprintf (mem, mem_size, fmt, args);
#ifdef UFO_WINDOWS
    #pragma warning(default: 4996)
#endif
        va_end (args);
        if ((res > 0) && (res < mem_size))
        {
            return res;
        }
        else
        {
            mem[mem_size - 1] = 0;
            return 1 - mem_size;
        }
    }
    return 0;
}
//------------------------------------------------------------------------------
inline uword mem_printf_written (int ret)
{
    return ret & ~((-1) >> 1);
}
//------------------------------------------------------------------------------

}

#endif /* UFO_UFO_MEM_PRINTF_HPP_ */

