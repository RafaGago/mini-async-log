/*
 * log_frontend.hpp
 *
 *  Created on: Nov 13, 2014
 *      Author: rafgag
 */


#ifndef TINY_LOG_LOG_FRONTEND_HPP_
#define TINY_LOG_LOG_FRONTEND_HPP_

#include <memory>
#include <tiny/message_encoder.hpp>

namespace tiny {

struct backend_cfg;

//------------------------------------------------------------------------------
class frontend
{
public:
    //--------------------------------------------------------------------------
    enum init_status
    {
        init_ok,
        init_done_by_other,
        init_other_failed,
        init_tried_but_failed,
        init_was_terminated
    };
    //--------------------------------------------------------------------------
    frontend();
    //--------------------------------------------------------------------------
    ~frontend();
    //--------------------------------------------------------------------------
    backend_cfg get_backend_cfg();
    //--------------------------------------------------------------------------
    init_status init_backend (const backend_cfg& cfg);
    //--------------------------------------------------------------------------
    void set_severity (sev::severity s);
    //--------------------------------------------------------------------------
    sev::severity severity();
    //--------------------------------------------------------------------------
    bool set_console_severity(
            sev::severity stderr, sev::severity stdout = sev::off
            );
    //--------------------------------------------------------------------------
    proto::encoder get_encoder (uword required_bytes);
    //--------------------------------------------------------------------------
    void push_encoded (proto::encoder encoder);
    //--------------------------------------------------------------------------
    void on_termination();                                                      //you may want to call this from e.g. SIGTERM handlers, be aware that all the data generators should be stopped before.
    //--------------------------------------------------------------------------
private:
    class frontend_impl;
    std::unique_ptr<frontend_impl> m;

}; //class log_backed
//------------------------------------------------------------------------------
} //namespaces

#endif /* TINY_LOG_LOG_FRONTEND_HPP_ */
