/*
 * system.hpp
 *
 *  Created on: Nov 21, 2014
 *      Author: rafgag
 */


#ifndef UFO_SYSTEM_HPP_
#define UFO_SYSTEM_HPP_

#if defined (__GXX_EXPERIMENTAL_CXX0X__) &&\
    defined (__GNUC__) &&\
    (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
    #define UFO_HAS_CONSTEXPR 1
    #define UFO_HAS_VARIADIC_TEMPLATES 1
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

#endif /* UFO_SYSTEM_HPP_ */
