/*
 * chrono.hpp
 *
 *  Created on: Nov 11, 2014
 *      Author: rafgag
 */


#ifndef UFO_CHRONO_HPP_
#define UFO_CHRONO_HPP_

#ifdef UFO_USE_BOOST_CHRONO

#include <boost/chrono.hpp>

#define UFO_CHRONO_NAMESPACE      ::boost::chrono
#define UFO_CHRONO_BASE_NAMESPACE ::boost  //for. e.g boost::micro

#else

#include <chrono>

#define UFO_CHRONO_NAMESPACE      ::std::chrono
#define UFO_CHRONO_BASE_NAMESPACE ::std  //for. e.g std::micro

#endif

#endif /* UFO_CHRONO_HPP_ */
