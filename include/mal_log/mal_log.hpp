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

#ifndef MAL_LOG_LOG_INCLUDE_HPP_
#define MAL_LOG_LOG_INCLUDE_HPP_

#include <mal_log/mal_strip.hpp>
#include <mal_log/mal_private.hpp>
#include <mal_log/mal_interface.hpp>
#include <mal_log/frontend_types.hpp>

//------------------------------------------------------------------------------
#ifndef MAL_GET_LOGGER_INSTANCE_FUNCNAME
    #define MAL_GET_LOGGER_INSTANCE_FUNC get_mal_logger_instance()
#else
    #define MAL_GET_LOGGER_INSTANCE_FUNC MAL_GET_LOGGER_INSTANCE_FUNCNAME()
#endif
//------------------------------------------------------------------------------
#define log_if(condition, statement) \
     MAL_LOG_IF_PRIVATE ((condition), (statement))
//------------------------------------------------------------------------------
#define log_every(count, statement) \
    MAL_LOG_EVERY_PRIVATE (__LINE__, (count), (statement))
//------------------------------------------------------------------------------
#ifndef MAL_STRIP_LOG_FILELINE
    #define log_fileline MAL_LOG_FILELINE_CONCAT_PRIVATE (__FILE__, __LINE__)
#else
    #define log_fileline
#endif
//------------------------------------------------------------------------------
#define log_fln log_fileline
//------------------------------------------------------------------------------
#ifndef MAL_STRIP_LOG_DEBUG

#define log_debug_i(instance, ...)\
    MAL_LOG_PRIVATE ((instance), true, debug, __VA_ARGS__)

#define log_debug_sync_i(instance, ...)\
    MAL_LOG_PRIVATE ((instance), false, debug, __VA_ARGS__)

#define log_debug(...)\
    log_debug_i (MAL_GET_LOGGER_INSTANCE_FUNC, __VA_ARGS__)

#define log_debug_sync(...)\
    log_debug_sync_i (MAL_GET_LOGGER_INSTANCE_FUNC, __VA_ARGS__)

#define log_debug_i_if(condition, instance, ...)\
    log_if ((condition), log_debug_i ((instance), __VA_ARGS__))

#define log_debug_sync_i_if(condition, instance, ...)\
    log_if ((condition), log_debug_sync_i ((instance), __VA_ARGS__))

#define log_debug_if(condition, ...)\
    log_if ((condition), log_debug (__VA_ARGS__))

#define log_debug_sync_if(condition, ...)\
    log_if ((condition), log_debug_sync (__VA_ARGS__))

#else

    #define log_debug_i(...)         ::mal::macro::silence_warnings()
    #define log_debug_sync_i(...)    ::mal::macro::silence_warnings()
    #define log_debug(...)           ::mal::macro::silence_warnings()
    #define log_debug_sync(...)      ::mal::macro::silence_warnings()
    #define log_debug_i_if(...)      ::mal::macro::silence_warnings()
    #define log_debug_sync_i_if(...) ::mal::macro::silence_warnings()
    #define log_debug_if(...)        ::mal::macro::silence_warnings()
    #define log_debug_sync_if(...)   ::mal::macro::silence_warnings()

#endif
//------------------------------------------------------------------------------
#ifndef MAL_STRIP_LOG_TRACE

#define log_trace_i(instance, ...)\
    MAL_LOG_PRIVATE ((instance), true, trace, __VA_ARGS__)

#define log_trace_sync_i(instance, ...)\
    MAL_LOG_PRIVATE ((instance), false, trace, __VA_ARGS__)

#define log_trace(...)\
    log_trace_i (MAL_GET_LOGGER_INSTANCE_FUNC, __VA_ARGS__)

#define log_trace_sync(...)\
    log_trace_sync_i (MAL_GET_LOGGER_INSTANCE_FUNC, __VA_ARGS__)

#define log_trace_i_if(condition, instance, ...)\
    log_if ((condition), log_trace_i ((instance), __VA_ARGS__))

#define log_trace_sync_i_if(condition, instance, ...)\
    log_if ((condition), log_trace_sync_i ((instance), __VA_ARGS__))

#define log_trace_if(condition, ...)\
    log_if ((condition), log_trace (__VA_ARGS__))

#define log_trace_sync_if(condition, ...)\
    log_if ((condition), log_trace_sync (__VA_ARGS__))

#else

    #define log_trace_i(...)         ::mal::macro::silence_warnings()
    #define log_trace_sync_i(...)    ::mal::macro::silence_warnings()
    #define log_trace(...)           ::mal::macro::silence_warnings()
    #define log_trace_sync(...)      ::mal::macro::silence_warnings()
    #define log_trace_i_if(...)      ::mal::macro::silence_warnings()
    #define log_trace_sync_i_if(...) ::mal::macro::silence_warnings()
    #define log_trace_if(...)        ::mal::macro::silence_warnings()
    #define log_trace_sync_if(...)   ::mal::macro::silence_warnings()

#endif
//------------------------------------------------------------------------------
#ifndef MAL_STRIP_LOG_NOTICE

#define log_notice_i(instance, ...)\
    MAL_LOG_PRIVATE ((instance), true, notice, __VA_ARGS__)

#define log_notice_sync_i(instance, ...)\
    MAL_LOG_PRIVATE ((instance), false, notice, __VA_ARGS__)

#define log_notice(...)\
    log_notice_i (MAL_GET_LOGGER_INSTANCE_FUNC, __VA_ARGS__)

#define log_notice_sync(...)\
    log_notice_sync_i (MAL_GET_LOGGER_INSTANCE_FUNC, __VA_ARGS__)

#define log_notice_i_if(condition, instance, ...)\
    log_if ((condition), log_notice_i ((instance), __VA_ARGS__))

#define log_notice_sync_i_if(condition, instance, ...)\
    log_if ((condition), log_notice_sync_i ((instance), __VA_ARGS__))

#define log_notice_if(condition, ...)\
    log_if ((condition), log_notice (__VA_ARGS__))

#define log_notice_sync_if(condition, ...)\
    log_if ((condition), log_notice_sync (__VA_ARGS__))

#else

    #define log_notice_i(...)         ::mal::macro::silence_warnings()
    #define log_notice_sync_i(...)    ::mal::macro::silence_warnings()
    #define log_notice(...)           ::mal::macro::silence_warnings()
    #define log_notice_sync(...)      ::mal::macro::silence_warnings()
    #define log_notice_i_if(...)      ::mal::macro::silence_warnings()
    #define log_notice_sync_i_if(...) ::mal::macro::silence_warnings()
    #define log_notice_if(...)        ::mal::macro::silence_warnings()
    #define log_notice_sync_if(...)   ::mal::macro::silence_warnings()

#endif
//------------------------------------------------------------------------------
#ifndef MAL_STRIP_LOG_WARNING

#define log_warning_i(instance, ...)\
    MAL_LOG_PRIVATE ((instance), true, warning, __VA_ARGS__)

#define log_warning_sync_i(instance, ...)\
    MAL_LOG_PRIVATE ((instance), false, warning, __VA_ARGS__)

#define log_warning(...)\
    log_warning_i (MAL_GET_LOGGER_INSTANCE_FUNC, __VA_ARGS__)

#define log_warning_sync(...)\
    log_warning_sync_i (MAL_GET_LOGGER_INSTANCE_FUNC, __VA_ARGS__)

#define log_warning_i_if(condition, instance, ...)\
    log_if ((condition), log_warning_i ((instance), __VA_ARGS__))

#define log_warning_sync_i_if(condition, instance, ...)\
    log_if ((condition), log_warning_sync_i ((instance), __VA_ARGS__))

#define log_warning_if(condition, ...)\
    log_if ((condition), log_warning (__VA_ARGS__))

#define log_warning_sync_if(condition, ...)\
    log_if ((condition), log_warning_sync (__VA_ARGS__))

#else

    #define log_warning_i(...)         ::mal::macro::silence_warnings()
    #define log_warning_sync_i(...)    ::mal::macro::silence_warnings()
    #define log_warning(...)           ::mal::macro::silence_warnings()
    #define log_warning_sync(...)      ::mal::macro::silence_warnings()
    #define log_warning_i_if(...)      ::mal::macro::silence_warnings()
    #define log_warning_sync_i_if(...) ::mal::macro::silence_warnings()
    #define log_warning_if(...)        ::mal::macro::silence_warnings()
    #define log_warning_sync_if(...)   ::mal::macro::silence_warnings()

#endif
//------------------------------------------------------------------------------
#ifndef MAL_STRIP_LOG_ERROR

#define log_error_i(instance, ...)\
    MAL_LOG_PRIVATE ((instance), true, error, __VA_ARGS__)

#define log_error_sync_i(instance, ...)\
    MAL_LOG_PRIVATE ((instance), false, error, __VA_ARGS__)

#define log_error(...)\
    log_error_i (MAL_GET_LOGGER_INSTANCE_FUNC, __VA_ARGS__)

#define log_error_sync(...)\
    log_error_sync_i (MAL_GET_LOGGER_INSTANCE_FUNC, __VA_ARGS__)

#define log_error_i_if(condition, instance, ...)\
    log_if ((condition), log_error_i ((instance), __VA_ARGS__))

#define log_error_sync_i_if(condition, instance, ...)\
    log_if ((condition), log_error_sync_i ((instance), __VA_ARGS__))

#define log_error_if(condition, ...)\
    log_if ((condition), log_error (__VA_ARGS__))

#define log_error_sync_if(condition, ...)\
    log_if ((condition), log_error_sync (__VA_ARGS__))

#else

    #define log_error_i(...)         ::mal::macro::silence_warnings()
    #define log_error_sync_i(...)    ::mal::macro::silence_warnings()
    #define log_error(...)           ::mal::macro::silence_warnings()
    #define log_error_sync(...)      ::mal::macro::silence_warnings()
    #define log_error_i_if(...)      ::mal::macro::silence_warnings()
    #define log_error_sync_i_if(...) ::mal::macro::silence_warnings()
    #define log_error_if(...)        ::mal::macro::silence_warnings()
    #define log_error_sync_if(...)   ::mal::macro::silence_warnings()

#endif
//------------------------------------------------------------------------------
#ifndef MAL_STRIP_LOG_CRITICAL

#define log_critical_i(instance, ...)\
    MAL_LOG_PRIVATE ((instance), true, critical, __VA_ARGS__)

#define log_critical_sync_i(instance, ...)\
    MAL_LOG_PRIVATE ((instance), false, critical, __VA_ARGS__)

#define log_critical(...)\
    log_critical_i (MAL_GET_LOGGER_INSTANCE_FUNC, __VA_ARGS__)

#define log_critical_sync(...)\
    log_critical_sync_i (MAL_GET_LOGGER_INSTANCE_FUNC, __VA_ARGS__)

#define log_critical_i_if(condition, instance, ...)\
    log_if ((condition), log_critical_i ((instance), __VA_ARGS__))

#define log_critical_sync_i_if(condition, instance, ...)\
    log_if ((condition), log_critical_sync_i ((instance), __VA_ARGS__))

#define log_critical_if(condition, ...)\
    log_if ((condition), log_critical (__VA_ARGS__))

#define log_critical_sync_if(condition, ...)\
    log_if ((condition), log_critical_sync (__VA_ARGS__))

#else

    #define log_critical_i(...)         ::mal::macro::silence_warnings()
    #define log_critical_sync_i(...)    ::mal::macro::silence_warnings()
    #define log_critical(...)           ::mal::macro::silence_warnings()
    #define log_critical_sync(...)      ::mal::macro::silence_warnings()
    #define log_critical_i_if(...)      ::mal::macro::silence_warnings()
    #define log_critical_sync_i_if(...) ::mal::macro::silence_warnings()
    #define log_critical_if(...)        ::mal::macro::silence_warnings()
    #define log_critical_sync_if(...)   ::mal::macro::silence_warnings()

#endif
//------------------------------------------------------------------------------
#endif /* MAL_LOG_LOG_INCLUDE_HPP_ */
