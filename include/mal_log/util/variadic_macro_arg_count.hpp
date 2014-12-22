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

#ifndef VARIADIC_MACRO_ARG_COUNT_HPP_
#define VARIADIC_MACRO_ARG_COUNT_HPP_

#define MAL_COUNT_VA_ARGS_PRIVATE(unused,\
            a1,a2,a3,a4,a5,a6,a7,a8,a9,\
            a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,\
            a20,a21,a22,a23,a24,a25,a26,a27,a28,a29,\
            a30,a31,a32,a33,a34,a35,a36,a37,a38,a39,\
            a40,a41,a42,a43,a44,a45,a46,a47,a48,a49,\
            a50,a51,a52,a53,a54,a55,a56,a57,a58,a59,\
            a60,a61,a62,a63,a64,a65,a66,a67,a68,a69,\
            a70,a71,a72,a73,a74,a75,a76,a77,a78,a79,\
            a80,a81,a82,a83,a84,a85,a86,a87,a88,a89,\
            a90,a91,a92,a93,a94,a95,a96,a97,a98,a99,\
            count,...\
            ) count

#define MAL_COUNT_VA_ARGS(...)\
        MAL_COUNT_VA_ARGS_PRIVATE (,##__VA_ARGS__,\
            99,98,97,96,95,94,93,92,91,90,\
            89,88,87,86,85,84,83,82,81,80,\
            79,78,77,76,75,74,73,72,71,70,\
            69,68,67,66,65,64,63,62,61,60,\
            59,58,57,56,55,54,53,52,51,50,\
            49,48,47,46,45,44,43,42,41,40,\
            39,38,37,36,35,34,33,32,31,30,\
            29,28,27,26,25,24,23,22,21,20,\
            19,18,17,16,15,14,13,12,11,10,\
            9,8,7,6,5,4,3,2,1,0\
            )

#endif /* VARIADIC_MACRO_ARG_COUNT_HPP_ */
