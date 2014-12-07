/*
 * opaque_pod.hpp
 *
 *  Created on: Dec 7, 2014
 *      Author: rafa
 */

#ifndef UFO_LOG_OPAQUE_POD_HPP_
#define UFO_LOG_OPAQUE_POD_HPP_

#include <type_traits>
#include <cstring>

namespace ufo {

//------------------------------------------------------------------------------
template<uword bytes>
class opaque_pod
{
public:
    //--------------------------------------------------------------------------
    template <class T>
    void write (T val)
    {
        static_assert (sizeof (T) <= bytes, "not enough storage for this type");
        std::memcpy (&m_stor, &val, sizeof val);
    }
    //--------------------------------------------------------------------------
    template <class T>
    T read_as()
    {
        static_assert (sizeof (T) <= bytes, "reading bigger than storage type");
        T val;
        std::memcpy (&val, &m_stor, sizeof val);
        return val;
    }
    //--------------------------------------------------------------------------
private:
   typename std::aligned_storage<bytes>::type m_stor;
};
//------------------------------------------------------------------------------
} //namespaces

#endif /* UFO_LOG_OPAQUE_POD_HPP_ */
