
/*
The BSD 3-clause license
--------------------------------------------------------------------------------
Copyright (c) 2017 Rafael Gago Castano. All rights reserved.

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
#ifndef MAL_PROCESSOR_PAUSE_HPP_
#define MAL_PROCESSOR_PAUSE_HPP_
/*----------------------------------------------------------------------------*/
#ifdef _MSC_VER
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #ifndef NOMINMAX
    #define NOMINMAX
  #endif
  #include <windows.h>
  #if WINVER < 0x0600
    #error "Update this code. YieldProcessor may not be available"
  #endif
  #define processor_pause() YieldProcessor()
#endif
/*---------------------------------------------------------------------------*/
#if defined (__GNUC__) || defined (GCC)

  /*When looking to add platforms just search the "cpu_relax()" function on
    the Linux Kernel tree for hints.*/
  #if defined (__i386__) || defined (__x86_64__)
    /*same opcode than PAUSE (_mm_pause): F3 90*/
    #define processor_pause()  __asm__ __volatile__ ("rep;nop": : :"memory")
  #endif
  #if defined (__arm__)
    /*will execute as NOP on processors that doesn't support it:
       http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0489f/CIHEGBBF.html
     */
    #define processor_pause() __asm__ __volatile__ ("yield": : :"memory")
  #endif
#endif
/*---------------------------------------------------------------------------*/
#if !defined (processor_pause)
  #error
    "processor_pause() unavailable (minimal impl is a compiler barrier)"
#if 0
  /*defined (__GNUC__) || defined (GCC)*/
  #define compiler_barrier() __asm__ __volatile__ ("" : : : "memory")
#endif
#endif
/*---------------------------------------------------------------------------*/
#endif /* __MAL_PROCESSOR_PAUSE_H__ */