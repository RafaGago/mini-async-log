/*
 * thread.hpp
 *
 *  Created on: Nov 11, 2014
 *      Author: rafgag
 */


#ifndef TINY_THREAD_HPP_
#define TINY_THREAD_HPP_

#ifdef TINY_USE_BOOST_THREAD

#include <boost/thread.hpp>

#define TINY_THREAD_NAMESPACE ::boost

#else

#include <thread>

#define TINY_THREAD_NAMESPACE ::std

#endif

#endif /* TINY_THREAD_HPP_ */
