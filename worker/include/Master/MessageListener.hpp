#ifndef _MESSAGE_LISTENER_HPP_INCLUDE_
#define _MESSAGE_LISTENER_HPP_INCLUDE_

#include <string>
#include "MasterRequest.hpp"

namespace Master {

class MessageListener {
public:
    virtual int OnMessage(MasterRequest *r) = 0;
    virtual void OnAborted(MasterRequest *r) = 0;
};
}

#endif
