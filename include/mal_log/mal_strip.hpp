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

#ifndef MAL_LOG_MAL_STRIP_HPP_
#define MAL_LOG_MAL_STRIP_HPP_

#ifdef MAL_STRIP_LOG_DEBUG
    #ifndef MAL_STRIP_LOG_SEVERITY
        #define MAL_STRIP_LOG_SEVERITY 0
    #else
        #error "use MAL_STRIP_LOG_SEVERITY or MAL_STRIP_LOG_DEBUG, not both"
    #endif
#endif

#ifdef MAL_STRIP_LOG_TRACE
    #ifndef MAL_STRIP_LOG_SEVERITY
        #define MAL_STRIP_LOG_SEVERITY 1
    #else
        #error "use MAL_STRIP_LOG_SEVERITY or MAL_STRIP_LOG_TRACE, not both"
    #endif
#endif

#ifdef MAL_STRIP_LOG_NOTICE
    #ifndef MAL_STRIP_LOG_SEVERITY
        #define MAL_STRIP_LOG_SEVERITY 2
    #else
        #error "use MAL_STRIP_LOG_SEVERITY or MAL_STRIP_LOG_NOTICE, not both"
    #endif
#endif

#ifdef MAL_STRIP_LOG_WARNING
    #ifndef MAL_STRIP_LOG_SEVERITY
        #define MAL_STRIP_LOG_SEVERITY 3
    #else
        #error "use MAL_STRIP_LOG_SEVERITY or MAL_STRIP_LOG_WARNING, not both"
    #endif
#endif

#ifdef MAL_STRIP_LOG_ERROR
    #ifndef MAL_STRIP_LOG_SEVERITY
        #define MAL_STRIP_LOG_SEVERITY 4
    #else
        #error "use MAL_STRIP_LOG_SEVERITY or MAL_STRIP_LOG_ERROR, not both"
    #endif
#endif

#if defined (MAL_STRIP_LOG_CRITICAL) || defined (MAL_STRIP_ALL)
    #ifndef MAL_STRIP_LOG_SEVERITY
        #define MAL_STRIP_LOG_SEVERITY 5
    #else
        #error "use MAL_STRIP_LOG_SEVERITY or MAL_STRIP_LOG_CRITICAL, not both"
    #endif
#endif

#if defined(MAL_STRIP_LOG_SEVERITY) &&\
    MAL_STRIP_LOG_SEVERITY >= 0 &&\
    !defined (MAL_STRIP_LOG_DEBUG)
        #define MAL_STRIP_LOG_DEBUG
#endif

#if defined(MAL_STRIP_LOG_SEVERITY) &&\
    MAL_STRIP_LOG_SEVERITY >= 1 &&\
    !defined (MAL_STRIP_LOG_TRACE)
        #define MAL_STRIP_LOG_TRACE
#endif

#if defined(MAL_STRIP_LOG_SEVERITY) &&\
    MAL_STRIP_LOG_SEVERITY >= 2 &&\
    !defined (MAL_STRIP_LOG_NOTICE)
        #define MAL_STRIP_LOG_NOTICE
#endif

#if defined(MAL_STRIP_LOG_SEVERITY) &&\
    MAL_STRIP_LOG_SEVERITY >= 3 &&\
    !defined (MAL_STRIP_LOG_WARNING)
        #define MAL_STRIP_LOG_WARNING
#endif

#if defined(MAL_STRIP_LOG_SEVERITY) &&\
    MAL_STRIP_LOG_SEVERITY >= 4 &&\
    !defined (MAL_STRIP_LOG_ERROR)
        #define MAL_STRIP_LOG_ERROR
#endif

#if defined(MAL_STRIP_LOG_SEVERITY) &&\
    MAL_STRIP_LOG_SEVERITY >= 5 &&\
    !defined (MAL_STRIP_LOG_CRITICAL)
        #define MAL_STRIP_LOG_CRITICAL
#endif

#endif /* MAL_LOG_MAL_STRIP_HPP_ */
