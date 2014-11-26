/*
The BSD 3-clause license
--------------------------------------------------------------------------------
Copyright (c) 2013-2014 Rafael Gago Castano. All rights reserved.

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

#ifndef UFO_LOG_LOG_FILES_HPP_
#define UFO_LOG_LOG_FILES_HPP_

#include <cstdio>
#include <cstring>
#include <string>
#include <deque>
#include <fstream>
#include <vector>

#include <ufo_log/util/integer.hpp>
#include <ufo_log/util/atomic.hpp>
#include <ufo_log/util/raw_circular_buffer.hpp>
#include <ufo_log/util/thread.hpp>

namespace ufo {

namespace th = UFO_THREAD_NAMESPACE;

//------------------------------------------------------------------------------
class log_files
{
public:
    //--------------------------------------------------------------------------
    bool init(
        uword                          file_count,
        const std::string&             folder,
        const std::string&             prefix,
        const std::string&             suffix,
        const std::deque<std::string>& previous
        )
    {
        assert (file_count == 0 || file_count > 1);
        uword max_name = folder.size() +
                         prefix.size() +
                         fixed_chars_in_name +
                         suffix.size();
        for (uword i = 0; i < previous.size(); ++i)
        {
            max_name = (previous[i].size() > max_name) ?
                            previous[i].size() : max_name;
        }
        ++max_name;                                                             //trailing null
        try
        {
            m_buffer.resize (max_name, 0);
            auto prev = previous;
            while (file_count && ((prev.size() >= file_count)))
            {
                erase_file (prev.front().c_str());
                prev.pop_front();
            }
            m_folder = folder;
            m_prefix = prefix;
            m_suffix = suffix;
        }
        catch (...)
        {
            assert (false && "log: memory exception");
            return false;
        }
        if (file_count)
        {
            m_name_strings.free();
            if (!m_name_strings.init (max_name, file_count))
            {
                return false;
            }
        }
        return true;
    }
    //--------------------------------------------------------------------------
    bool can_write_in_folder (const std::string& folder)
    {
        try
        {
            if (m_buffer.size() < (folder.size() + fixed_chars_in_name + 1))
            {
                m_buffer.resize (folder.size() + fixed_chars_in_name + 1, -1);
            }
            for (uword i = 0; ; ++i)
            {
                std::string empty;
                new_file_name_c_str_in_buffer (folder, empty, empty, i ,i + 1);
                const char* fn = filename_in_buffer();
                std::ofstream file (fn);
                file.write ((const char*) &fn, m_buffer.size());
                if (file.good())
                {
                    file.close();
                    erase_file (fn);
                    break;
                }
                erase_file (fn);
                if (i == 20) //FIXME
                {
                    assert(
                        false && "log: couldn't create or write a test file"
                        );
                    return false;
                }
                th::this_thread::sleep_for (ch::milliseconds (1));
            }
        }
        catch (...)
        {
            assert (false && "log: memory exception");
            return false;
        }
        return true;
    }
    //--------------------------------------------------------------------------
    const char* new_filename_in_buffer (u64 cpu, u64 calendar)
    {
        new_file_name_c_str_in_buffer(
                m_folder, m_prefix, m_suffix, cpu, calendar
                );
        return filename_in_buffer();
    }
    //--------------------------------------------------------------------------
    const char* filename_in_buffer()
    {
        return &m_buffer[0];
    }
    //--------------------------------------------------------------------------
    void push_filename_in_buffer()
    {
        assert (!m_name_strings.is_full());
        m_name_strings.push_tail();
        std::memcpy (m_name_strings.tail(), &m_buffer[0], m_buffer.size());
    }
    //--------------------------------------------------------------------------
    bool rotates() const { return m_name_strings.is_initialized(); }
    //--------------------------------------------------------------------------
    void keep_newer_files (uword keep_count)
    {
        assert (rotates());
        while (m_name_strings.size() > keep_count)
        {
            erase_file ((const char*) m_name_strings.head());
            m_name_strings.pop_head();
        }
    }
    //--------------------------------------------------------------------------
private:
    //--------------------------------------------------------------------------
    static void erase_file (const char* file)
    {
        std::remove (file);
    }
    //--------------------------------------------------------------------------
    void new_file_name_c_str_in_buffer(
            const std::string& folder,
            const std::string& prefix,
            const std::string& suffix,
            u64                cpu,
            u64                calendar
            )
    {
        assert (m_buffer.size() >=
                (folder.size() +
                 prefix.size() +
                 suffix.size() +
                 fixed_chars_in_name +
                 1
                 )
                );
        char* str = &m_buffer[0];
        auto sz   = folder.size();
        std::memcpy (str, &folder[0], sz);
        str      += sz;

        sz        = prefix.size();
        std::memcpy (str, &prefix[0], sz);
        str      += sz;

        sz        = fixed_chars_in_name;
        snprintf (str, sz + 1, "%016lx-%016lx", calendar, cpu);
        str      += sz;

        sz        = suffix.size();
        std::memcpy (str, &suffix[0], sz);
        str      += sz;

        *str      = 0;
    }
    //--------------------------------------------------------------------------
    static const uword fixed_chars_in_name = 16 + 1 + 16;
    //--------------------------------------------------------------------------
    std::string         m_folder, m_prefix, m_suffix;
    std::vector<char>   m_buffer;
    raw_circular_buffer m_name_strings;
    //--------------------------------------------------------------------------
};
//------------------------------------------------------------------------------
} //namespaces

#endif /* UFO_LOG_LOG_FILES_HPP_ */
