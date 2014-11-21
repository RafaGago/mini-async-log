/*
 * chrono.hpp
 *
 *  Created on: Nov 11, 2014
 *      Author: rafgag
 */


#ifndef TINY_CHRONO_HPP_
#define TINY_CHRONO_HPP_

#ifdef TINY_USE_BOOST_CHRONO

#include <boost/chrono.hpp>

#define TINY_CHRONO_NAMESPACE      ::boost::chrono
#define TINY_CHRONO_BASE_NAMESPACE ::boost  //for. e.g boost::micro

#else

#include <chrono>

#define TINY_CHRONO_NAMESPACE      ::std::chrono
#define TINY_CHRONO_BASE_NAMESPACE ::std  //for. e.g std::micro

#endif

#endif /* TINY_CHRONO_HPP_ */
