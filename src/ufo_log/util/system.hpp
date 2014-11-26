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
#ifndef UFO_SYSTEM_HPP_
#define UFO_SYSTEM_HPP_

#if defined (__GXX_EXPERIMENTAL_CXX0X__) &&\
    defined (__GNUC__) &&\
    (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
    #define UFO_HAS_CONSTEXPR 1
    #define UFO_HAS_VARIADIC_TEMPLATES 1

    #if __x86_64__ || __ppc64__
        #define UFO_64
    #else
        #define UFO_32
    #endif
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

#if _MSC_VER >= 1800 //1600 >= vs2010
    #define UFO_HAS_VARIADIC_TEMPLATES 1

    #ifdef _WIN64
        #define UFO_64
    #else
        #define UFO_32
    #endif
#endif

#endif

#ifndef UFO_DYN_LIB_CALL

#if defined (_MSC_VER)
    #define UFO_DYN_LIB_CALL WINAPI
#elif __GNUC__ >= 4
    #define UFO_DYN_LIB_CALL __attribute__ ((visibility ("default")))
#else
    #define UFO_DYN_LIB_CALL
#endif

#endif //UFO_DYN_LIB_CALL

namespace ufo {
#ifndef UFO_CACHE_LINE_SIZE
    const unsigned cache_line_size = 64;
#else
    const unsigned cache_line_size = UFO_CACHE_LINE_SIZE;
#endif
} //namespace ufo {


#if defined (UFO_HAS_CONSTEXPR) && defined (UFO_HAS_VARIADIC_TEMPLATES)
    #define UFO_COMPILE_TIME_FMT_CHECK
#endif

#endif /* UFO_SYSTEM_HPP_ */
