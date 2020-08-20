#pragma once

#include "NetServer.hpp"
#include "Filter.hpp"
#include "MasterProcess.hpp"

namespace Master {

class WebRtcService : public NetServer::Listener , public Filter {
public:
    WebRtcService(NetServer *messageServer);
    virtual ~WebRtcService();

public:
    void SetMaster(MasterProcess *master);

// filter callbak
public:
    virtual int OnPlay(RtcSession *s, PlayValue *val) override;
    virtual int OnPublish(RtcSession *s, PublishValue *val) override;
    virtual int OnCloseStream(RtcSession *s, CloseStreamValue *val) override;

// message callback
public:
    virtual int OnMessage(NetRequest *request) override;
    virtual void OnAborted(NetRequest *request) override;

private:
    MasterProcess *m_master;
    NetServer *m_messageServer;
};

}
