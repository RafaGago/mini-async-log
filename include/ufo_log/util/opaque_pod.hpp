/*
 * opaque_pod.hpp
 *
 *  Created on: Dec 7, 2014
 *      Author: rafa
 */

#ifndef MAL_LOG_OPAQUE_POD_HPP_
#define MAL_LOG_OPAQUE_POD_HPP_

#include <type_traits>
#include <cstring>

namespace ufo {

//------------------------------------------------------------------------------
template<uword payload_bytes>
class opaque_pod
{
public:
    //--------------------------------------------------------------------------
    static const uword bytes = payload_bytes;
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

#endif /* MAL_LOG_OPAQUE_POD_HPP_ */
