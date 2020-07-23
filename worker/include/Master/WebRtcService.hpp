#pragma once

#include "MessageListener.hpp"
#include "Filter.hpp"

namespace Master {

class WebRtcService : public MessageListener , public Filter {
public:
    WebRtcService();
    virtual ~WebRtcService();

// filter callbak
public:
    virtual int OnConnection(RtcSession *s, ConnectionValue *val) override;
    virtual int OnPlay(RtcSession *s, PlayValue *val) override;
    virtual int OnPublish(RtcSession *s, PublishValue *val) override;
    virtual int OnCloseStream(RtcSession *s, CloseStreamValue *val) override;

// message callback
public:
    virtual int OnMessage(MasterRequest *r) override;
    virtual void OnAborted(MasterRequest *r) override;

};

}
