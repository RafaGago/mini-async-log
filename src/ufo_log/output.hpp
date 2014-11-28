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

#ifndef UFO_LOG_LOG_OUTPUT_HPP_
#define UFO_LOG_LOG_OUTPUT_HPP_

#include <cstdio>
#include <cassert>
#include <fstream>
#include <iostream>
#include <ufo_log/util/integer.hpp>
#include <ufo_log/util/atomic.hpp>
#include <ufo_log/protocol.hpp>

namespace ufo {
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
        m_file.close();
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
        return m_file.tellp();
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
    void set_next_writes_severity (sev::severity s)
    {
        assert (s < sev::invalid);
        m_sev_current = s;
    }
    //--------------------------------------------------------------------------
    void write (const void* d, uword sz)
    {
        if (m_sev_current >= m_file_sev)
        {
            m_file.write ((const char*) d, sz);
        }
        if (m_sev_current >= m_stderr_sev)
        {
            std::cerr.write ((const char*) d, sz);
        }
        else if (m_sev_current >= m_stdout_sev)
        {
            std::cout.write ((const char*) d, sz);
        }
    }
    //--------------------------------------------------------------------------
    void write (const char* str)
    {
        raw_write (m_sev_current, str);
    }
    //--------------------------------------------------------------------------
    void raw_write (sev::severity s, const char* str)
    {
        if (s >= m_file_sev)
        {
            m_file << str;
        }
        if (s >= m_stderr_sev)
        {
            std::cerr << str;
        }
        else if (s >= m_stdout_sev)
        {
            std::cout << str;
        }
    }
    //--------------------------------------------------------------------------
private:
    sev::severity                    m_sev_current;
    mo_relaxed_atomic<sev::severity> m_stderr_sev;
    mo_relaxed_atomic<sev::severity> m_stdout_sev;
    mo_relaxed_atomic<sev::severity> m_file_sev;
    std::ofstream                    m_file;
};
//------------------------------------------------------------------------------
}

#endif /* UFO_LOG_LOG_FILE_SINK_HPP_ */
