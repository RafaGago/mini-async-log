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

#ifndef MAL_LOG_BOOST_FILESYSTEM_LIST_ALL_FILES_HPP_
#define MAL_LOG_BOOST_FILESYSTEM_LIST_ALL_FILES_HPP_

#include <boost/filesystem.hpp>
#include <mal_log/cfg.hpp>

//This file is not part of mal log, just a simple example of how to enable
//rotation between subsequent runs using boost filesystem.

//Be aware that it doesn't check for log file sizes or data placed by the user
//in the folder.

namespace mal { namespace extras {
//------------------------------------------------------------------------------
past_executions_file_list list_all_files (const char* path)
{
    namespace bfs = boost::filesystem;
    typedef bfs::directory_iterator dir_it;
    past_executions_file_list ret;
    for (dir_it it (path), end; it != end; ++it) {
        if (bfs::is_regular_file (*it) && !bfs::is_symlink (*it)) {
            ret.push_back (it->path().string());
        }
    }
    return ret;
}
//------------------------------------------------------------------------------
}} //namespaces

#endif /* MAL_LOG_BOOST_FILESYSTEM_LIST_ALL_FILES_HPP_ */
