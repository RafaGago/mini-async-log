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
#ifndef MAL_SYSTEM_HPP_
#define MAL_SYSTEM_HPP_

#if defined (__GXX_EXPERIMENTAL_CXX0X__) &&\
    defined (__GNUC__) &&\
    (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))

    #define MAL_HAS_CONSTEXPR 1
    #define MAL_HAS_VARIADIC_TEMPLATES 1
    #define MAL_UNIX_LIKE //this is wrong but correct enough for now to just support win an linux
    #define MAL_ALIGNED_STORAGE_DEFAULTS_MAX_ALIGN

    #if __x86_64__ || __ppc64__
        #define MAL_64
    #else
        #define MAL_32
    #endif

namespace mal {
    const char fs_separator = '/';
}

#endif

#if 0 //REMINDER

MSVC++ 12.0 _MSC_VER == 1800 (Visual Studio 2013)
MSVC++ 11.0 _MSC_VER == 1700 (Visual Studio 2012)
MSVC++ 10.0 _MSC_VER == 1600 (Visual Studio 2010)
MSVC++ 9.0  _MSC_VER == 1500 (Visual Studio 2008)
MSVC++ 8.0  _MSC_VER == 1400 (Visual Studio 2005)
MSVC++ 7.1  _MSC_VER == 1310 (Visual Studio 2003)

#endif

#if defined (_MSC_VER)

#ifdef _WIN64
    #define MAL_64
#else
    #define MAL_32
#endif

    #define MAL_WINDOWS

#if _MSC_VER >= 1800 //1600 >= vs2010
    #define MAL_HAS_VARIADIC_TEMPLATES 1
#endif

#if _MSC_VER >= 1700
    #define MAL_ALIGNED_STORAGE_DEFAULTS_MAX_ALIGN
#endif

namespace mal {
    const char fs_separator = '\\';
}

#endif

#ifndef MAL_DYN_LIB_CALL

#if defined (_MSC_VER)
    #define MAL_LIB_EXPORTED_FUNC WINAPI  //using def files
    #if defined (MAL_DYNLIB_COMPILE)
        #define MAL_LIB_EXPORTED_CLASS __declspec(dllexport)
    #else
        #define MAL_LIB_EXPORTED_CLASS __declspec(dllimport)
    #endif
#elif __GNUC__ >= 4
    #define MAL_LIB_EXPORTED_FUNC __attribute__ ((visibility ("default")))
    #define MAL_LIB_EXPORTED_CLASS __attribute__ ((visibility ("default")))
#else
    #define MAL_LIB_EXPORTED_FUNC
    #define MAL_LIB_EXPORTED_CLASS
#endif

#endif //MAL_DYN_LIB_CALL

namespace mal {
#ifndef MAL_CACHE_LINE_SIZE
    const unsigned cache_line_size = 64;
#else
    const unsigned cache_line_size = MAL_CACHE_LINE_SIZE;
#endif
} //namespace mal {

#if defined (MAL_HAS_CONSTEXPR)
    #define MAL_CONSTEXPR constexpr
#else
    #define MAL_CONSTEXPR constexpr
#endif

#endif /* MAL_SYSTEM_HPP_ */
