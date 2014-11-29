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

#ifndef UFO_LOG_LOG_INCLUDE_HPP_
#define UFO_LOG_LOG_INCLUDE_HPP_

#include <ufo_log/frontend_types.hpp>
#include <ufo_log/ufo_private.hpp>
#include <ufo_log/ufo_interface.hpp>

//------------------------------------------------------------------------------
#ifndef UFO_GET_LOGGER_INSTANCE_FUNCNAME
    #define UFO_GET_LOGGER_INSTANCE_FUNC get_ufo_logger_instance()
#else
    #define UFO_GET_LOGGER_INSTANCE_FUNC UFO_GET_LOGGER_INSTANCE_FUNCNAME()
#endif
//------------------------------------------------------------------------------
#define log_if(condition, statement) UFO_LOG_IF_PRIVATE (condition, statement)
//------------------------------------------------------------------------------
#define log_every(count, statement) \
    UFO_LOG_EVERY_PRIVATE (__LINE__, count, statement)
//------------------------------------------------------------------------------
#ifndef UFO_STRIP_LOG_FILELINE
    #define log_fileline UFO_LOG_FILELINE_CONCAT_PRIVATE (__FILE__, __LINE__)
#else
    #define log_fileline
#endif
//------------------------------------------------------------------------------
#define log_fln log_fileline
//------------------------------------------------------------------------------
#if !defined (UFO_STRIP_LOG_SEVERITY)

#define log_debug_i(instance, ...)\
    UFO_LOG_PRIVATE (instance, debug, __VA_ARGS__)

#define log_debug(...)\
    log_debug_i (UFO_GET_LOGGER_INSTANCE_FUNC, __VA_ARGS__)

#else

    #define log_debug_i(...) ::ufo::macro::silence_warnings()
    #define log_debug(...)   ::ufo::macro::silence_warnings()

#endif //UFO_STRIP_LOG_SEVERITY
//------------------------------------------------------------------------------
#if !defined (UFO_STRIP_LOG_SEVERITY) || UFO_STRIP_LOG_SEVERITY < 1

#define log_trace_i(instance, ...)\
    UFO_LOG_PRIVATE (instance, trace, __VA_ARGS__)

#define log_trace(...)\
    log_trace_i (UFO_GET_LOGGER_INSTANCE_FUNC, __VA_ARGS__)

#else

    #define log_trace_i(...) ::ufo::macro::silence_warnings()
    #define log_trace(...)   ::ufo::macro::silence_warnings()

#endif
//------------------------------------------------------------------------------
#if !defined (UFO_STRIP_LOG_SEVERITY) || UFO_STRIP_LOG_SEVERITY < 2

#define log_notice_i(instance, ...)\
    UFO_LOG_PRIVATE (instance, notice, __VA_ARGS__)

#define log_notice(...)\
    log_notice_i (UFO_GET_LOGGER_INSTANCE_FUNC, __VA_ARGS__)

#else

    #define log_notice_i(...) ::ufo::macro::silence_warnings()
    #define log_notice(...)   ::ufo::macro::silence_warnings()

#endif
//------------------------------------------------------------------------------
#if !defined (UFO_STRIP_LOG_SEVERITY) || UFO_STRIP_LOG_SEVERITY < 3

#define log_warning_i(instance, ...)\
    UFO_LOG_PRIVATE (instance, warning, __VA_ARGS__)

#define log_warning(...)\
    log_warning_i (UFO_GET_LOGGER_INSTANCE_FUNC, __VA_ARGS__)

#else

    #define log_warning_i(...) ::ufo::macro::silence_warnings()
    #define log_warning(...)   ::ufo::macro::silence_warnings()

#endif
//------------------------------------------------------------------------------
#if !defined (UFO_STRIP_LOG_SEVERITY) || UFO_STRIP_LOG_SEVERITY < 4

#define log_error_i(instance, ...)\
    UFO_LOG_PRIVATE (instance, error, __VA_ARGS__)

#define log_error(...)\
    log_error_i (UFO_GET_LOGGER_INSTANCE_FUNC, __VA_ARGS__)

#else

    #define log_error_i(...) ::ufo::macro::silence_warnings()
    #define log_error(...)   ::ufo::macro::silence_warnings()

#endif
//------------------------------------------------------------------------------
#if !defined (UFO_STRIP_LOG_SEVERITY) || UFO_STRIP_LOG_SEVERITY < 5

#define log_critical_i(instance, ...)\
    UFO_LOG_PRIVATE (instance, critical, __VA_ARGS__)

#define log_critical(...)\
    log_critical_i (UFO_GET_LOGGER_INSTANCE_FUNC, __VA_ARGS__)

#else

    #define log_critical_i(...) ::ufo::macro::silence_warnings()
    #define log_critical(...)   ::ufo::macro::silence_warnings()

#endif
//------------------------------------------------------------------------------
#endif /* UFO_LOG_LOG_INCLUDE_HPP_ */
