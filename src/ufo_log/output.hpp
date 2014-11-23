/*
 * log_file_sink.hpp
 *
 *  Created on: Nov 15, 2014
 *      Author: rafgag
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
    void set_console_severity (sev::severity stderr, sev::severity stdout)
    {
        assert (stdout < sev::invalid);
        assert (stderr < sev::invalid);
        assert ((stdout < stderr) || stdout == sev::off);
        m_stderr_sev = stderr;
        m_stdout_sev = stdout;
    }
    //--------------------------------------------------------------------------
    void next_writes_severity (sev::severity s)                             //just affects stdout and stderr, everything come to the backend ends up in the files
    {
        assert (s < sev::invalid);
        m_sev_current = s;
    }
    //--------------------------------------------------------------------------
    void write (const char* d, uword sz)
    {
        m_file.write (d, sz);
        if (m_sev_current >= m_stderr_sev)
        {
            std::cerr.write (d, sz);
        }
        else if (m_sev_current >= m_stdout_sev)
        {
            std::cout.write (d, sz);
        }
    }
    //--------------------------------------------------------------------------
    void write (const char* str)
    {
        m_file << str;
        if (m_sev_current >= m_stderr_sev)
        {
            std::cerr << str;
        }
        else if (m_sev_current >= m_stdout_sev)
        {
            std::cout << str;
        }
    }
    //--------------------------------------------------------------------------
private:
    sev::severity                    m_sev_current;
    mo_relaxed_atomic<sev::severity> m_stderr_sev;
    mo_relaxed_atomic<sev::severity> m_stdout_sev;
    std::ofstream                    m_file;
};
//------------------------------------------------------------------------------
}

#endif /* UFO_LOG_LOG_FILE_SINK_HPP_ */
