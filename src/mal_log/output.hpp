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

#ifndef MAL_LOG_LOG_OUTPUT_HPP_
#define MAL_LOG_LOG_OUTPUT_HPP_

#include <cstdio>
#include <cassert>
#include <fstream>
#include <iostream>
#include <mal_log/util/integer.hpp>
#include <mal_log/util/atomic.hpp>
#include <mal_log/frontend_types.hpp>

namespace mal {
//------------------------------------------------------------------------------
class output
{
public:
    //--------------------------------------------------------------------------
    output()
    {
        m_sev_current = sev::off;
        m_stderr_sev  = sev::off;
        m_stdout_sev  = sev::off;
        m_file_sev    = sev::warning;
    }
    //--------------------------------------------------------------------------
    bool file_open (const char* file)
    {
        m_file.open (file);
        return file_no_error();
    }
    //--------------------------------------------------------------------------
    bool file_is_open ()
    {
        return m_file.is_open();
    }
    //--------------------------------------------------------------------------
    void file_close()
    {
        if (file_is_open()) {
            m_file.close();
        }
    }
    //--------------------------------------------------------------------------
    bool file_no_error() const
    {
        return m_file.good();
    }
    //--------------------------------------------------------------------------
    void flush()
    {
        m_file.flush();
    }
    //--------------------------------------------------------------------------
    uword file_bytes_written()
    {
        return (uword) m_file.tellp();
    }
    //--------------------------------------------------------------------------
    void set_console_severity(
            sev::severity stderr_sev,
            sev::severity stdout_sev
            )
    {
        assert (stderr_sev < sev::invalid);
        assert (stdout_sev < sev::invalid);
        assert ((stdout_sev < stderr_sev) || stdout_sev == sev::off);
        m_stderr_sev = stderr_sev;
        m_stdout_sev = stdout_sev;
    }
    //--------------------------------------------------------------------------
    void set_file_severity (sev::severity sev)
    {
        assert (sev < sev::invalid);
        m_file_sev = sev;
    }
    //--------------------------------------------------------------------------
    sev::severity stderr_sev() const
    {
        return (sev::severity) m_stderr_sev;
    }
    //--------------------------------------------------------------------------
    sev::severity stdout_sev() const
    {
        return (sev::severity) m_stdout_sev;
    }
    //--------------------------------------------------------------------------
    sev::severity file_sev() const
    {
        return (sev::severity) m_file_sev;
    }
    //--------------------------------------------------------------------------
    sev::severity min_severity() const
    {
        sev::severity err, out, file;
        err   = stderr_sev();
        out   = stdout_sev();
        file  = file_sev();

        auto min = (err <= out)  ? err : out;
        min      = (min <= file) ? min : file;
        return min;
    }
    //--------------------------------------------------------------------------
    void entry_begin (sev::severity s)
    {
        assert (s < sev::invalid);
        m_sev_current = s;
    }
    //--------------------------------------------------------------------------
    void entry_end()
    {
        char newline = '\n';
        write (&newline, sizeof newline);
    }
    //--------------------------------------------------------------------------
    void write (const void* d, uword sz)
    {
        write_impl (m_sev_current, d, sz);
    }
    //--------------------------------------------------------------------------
    void write (const char* str)
    {
        raw_write (m_sev_current, str);
    }
    //--------------------------------------------------------------------------
    void raw_write (sev::severity s, const char* str)
    {
        static const uword max_str    = 2048;
        static const uword block_max  = 64;
        static const uword block_mask = block_max - 1;

        if (str == nullptr || str[0] == 0) { return; }

        uword i = 1;
        for (; i <= max_str; ++i) {
            if ((i & block_mask) == 0) {
                uword last_block_first = ((i - 1) & ~block_mask);
                write_impl (s, str + last_block_first, block_max);
            }
            if (str[i] == 0) {
                uword this_block_offset = i & block_mask;
                write_impl (s, str + i - this_block_offset, this_block_offset); //if /0 is the first character of the block I perform a 0 length write.
                return;
            }
        }
        if (i == max_str + 1) {
            static const char too_long[] = " [logger err]->string too long";
            write_impl (s, too_long, sizeof too_long - 1);
        }
    }
    //--------------------------------------------------------------------------
private:
    //--------------------------------------------------------------------------
    void write_impl (sev::severity s, const void* d, uword sz)
    {
        if (sz && d) {
            if (s >= m_file_sev) {
                m_file.write ((const char*) d, sz);
            }
            if (s >= m_stderr_sev) {
                std::cerr.write ((const char*) d, sz);
            }
            else if (s >= m_stdout_sev) {
                std::cout.write ((const char*) d, sz);
            }
        }
    }
    //--------------------------------------------------------------------------
    sev::severity                    m_sev_current;
    mo_relaxed_atomic<sev::severity> m_stderr_sev;
    mo_relaxed_atomic<sev::severity> m_stdout_sev;
    mo_relaxed_atomic<sev::severity> m_file_sev;
    std::ofstream                    m_file;
};
//------------------------------------------------------------------------------
}

#endif /* MAL_LOG_LOG_FILE_SINK_HPP_ */
