/*
 * system.hpp
 *
 *  Created on: Nov 21, 2014
 *      Author: rafgag
 */


#ifndef TINY_SYSTEM_HPP_
#define TINY_SYSTEM_HPP_

#if defined (__GXX_EXPERIMENTAL_CXX0X__) &&\
    defined (__GNUC__) &&\
    (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
    #define TINY_HAS_CONSTEXPR 1
    #define TINY_HAS_VARIADIC_TEMPLATES 1
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
    #define TINY_HAS_VARIADIC_TEMPLATES 1
#endif

#endif

#ifndef TINY_DYN_LIB_CALL

#if defined (_MSC_VER)
    #define TINY_DYN_LIB_CALL WINAPI
#elif __GNUC__ >= 4
    #define TINY_DYN_LIB_CALL __attribute__ ((visibility ("default")))
#else
    #define TINY_DYN_LIB_CALL
#endif

#endif //TINY_DYN_LIB_CALL

namespace tiny {
#ifndef TINY_CACHE_LINE_SIZE
    const unsigned cache_line_size = 64;
#else
    const unsigned cache_line_size = TINY_CACHE_LINE_SIZE;
#endif
} //namespace tiny {

#endif /* TINY_SYSTEM_HPP_ */
