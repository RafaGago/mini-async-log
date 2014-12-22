/*
 * opaque_pod.hpp
 *
 *  Created on: Dec 7, 2014
 *      Author: rafa
 */

#ifndef MAL_LOG_OPAQUE_POD_HPP_
#define MAL_LOG_OPAQUE_POD_HPP_

#include <mal_log/util/system.hpp>
#include <type_traits>
#include <cstring>

#ifndef MAL_ALIGNED_STORAGE_DEFAULTS_MAX_ALIGN
    #include <mal_log/util/max_align.hpp>
#endif

namespace mal {

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

#ifdef MAL_ALIGNED_STORAGE_DEFAULTS_MAX_ALIGN
    typename std::aligned_storage<bytes>::type m_stor;
#else //It might be better to just wrap our own aligned_storage type.
    typename std::aligned_storage<
                    bytes,
                    std::alignment_of<max_align_type_reinvent>::value
                    >::type m_stor;
#endif
};
//------------------------------------------------------------------------------
} //namespaces

#endif /* MAL_LOG_OPAQUE_POD_HPP_ */
