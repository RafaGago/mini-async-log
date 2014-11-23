/*
 * thread.hpp
 *
 *  Created on: Nov 11, 2014
 *      Author: rafgag
 */


#ifndef UFO_THREAD_HPP_
#define UFO_THREAD_HPP_

#ifdef UFO_USE_BOOST_THREAD

#include <boost/thread.hpp>

#define UFO_THREAD_NAMESPACE ::boost

#else

#include <thread>

#define UFO_THREAD_NAMESPACE ::std

#endif

#endif /* UFO_THREAD_HPP_ */
