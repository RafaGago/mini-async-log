/*
 * system.hpp
 *
 *  Created on: Nov 21, 2014
 *      Author: rafgag
 */


#ifndef TINY_SYSTEM_HPP_
#define TINY_SYSTEM_HPP_

namespace tiny {
#ifndef TINY_CACHE_LINE_SIZE
    const unsigned cache_line_size = 64;
#else
    const unsigned cache_line_size = TINY_CACHE_LINE_SIZE;
#endif
} //namespace tiny {

#endif /* TINY_SYSTEM_HPP_ */
