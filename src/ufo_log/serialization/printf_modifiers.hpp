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

#ifndef UFO_LOG_PRINTF_MODIFIERS_HPP_
#define UFO_LOG_PRINTF_MODIFIERS_HPP_

namespace ufo { namespace ser {
//------------------------------------------------------------------------------
struct u8_modif
{
    static const char* norm;
    static const char* fwidth_s;
    static const char* fwidth;
    static const char* hex;
};
const char* u8_modif::norm     = "%u";
const char* u8_modif::fwidth_s = "%3u";
const char* u8_modif::fwidth   = "%03u";
const char* u8_modif::hex      = "0x%02x";
//------------------------------------------------------------------------------
struct u16_modif
{
    static const char* norm;
    static const char* fwidth_s;
    static const char* fwidth;
    static const char* hex;
};
const char* u16_modif::norm     = "%hu";
const char* u16_modif::fwidth_s = "%5hu";
const char* u16_modif::fwidth   = "%05hu";
const char* u16_modif::hex      = "0x%04hx";
//------------------------------------------------------------------------------
struct u32_modif
{
    static const char* norm;
    static const char* fwidth_s;
    static const char* fwidth;
    static const char* hex;
};
const char* u32_modif::norm     = "%lu";
const char* u32_modif::fwidth_s = "%10lu";
const char* u32_modif::fwidth   = "%010lu";
const char* u32_modif::hex      = "0x%08lx";
//------------------------------------------------------------------------------
struct u64_modif
{
    static const char* norm;
    static const char* fwidth_s;
    static const char* fwidth;
    static const char* hex;
};
const char* u64_modif::norm     = "%llu";
const char* u64_modif::fwidth_s = "%2ollu";
const char* u64_modif::fwidth   = "%020llu";
const char* u64_modif::hex      = "0x%016llx";
//------------------------------------------------------------------------------
struct i8_modif
{
    static const char* norm;
    static const char* fwidth_s;
    static const char* fwidth;
    static const char* hex;
};
const char* i8_modif::norm     = "%d";
const char* i8_modif::fwidth_s = "%3d";
const char* i8_modif::fwidth   = "%03d";
const char* i8_modif::hex      = "0x%02x";
//------------------------------------------------------------------------------
struct i16_modif
{
    static const char* norm;
    static const char* fwidth_s;
    static const char* fwidth;
    static const char* hex;
};
#warning "fix all integer modifiers"
const char* i16_modif::norm     = "%hd";
const char* i16_modif::fwidth_s = "% 5hd";
const char* i16_modif::fwidth   = "%05hd";
const char* i16_modif::hex      = "0x%04hx";
//------------------------------------------------------------------------------
struct i32_modif
{
    static const char* norm;
    static const char* fwidth_s;
    static const char* fwidth;
    static const char* hex;
};
const char* i32_modif::norm     = "%ld";
const char* i32_modif::fwidth_s = "%5ld";
const char* i32_modif::fwidth   = "%05ld";
const char* i32_modif::hex      = "0x%08hx";
//------------------------------------------------------------------------------
struct i64_modif
{
    static const char* norm;
    static const char* fwidth_s;
    static const char* fwidth;
    static const char* hex;
};
const char* i64_modif::norm     = "%lld";
const char* i64_modif::fwidth_s = "%20lld";
const char* i64_modif::fwidth   = "%020lld";
const char* i64_modif::hex      = "0x%016llx";
//------------------------------------------------------------------------------
struct float_modif
{
    static const char* norm;
    static const char* sci;
    static const char* hex;
};
const char* float_modif::norm = "%g";
const char* float_modif::sci  = "%e";
const char* float_modif::hex  = "0x%08llx";
//------------------------------------------------------------------------------
struct double_modif
{
    static const char* norm;
    static const char* sci;
    static const char* hex;
};
const char* double_modif::norm = "%g";
const char* double_modif::sci  = "%e";
const char* double_modif::hex  = "0x%016llx";
//------------------------------------------------------------------------------
}} //namespaces

#endif /* UFO_LOG_PRINTF_MODIFIERS_HPP_ */
