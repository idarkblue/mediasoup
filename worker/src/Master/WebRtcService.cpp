#define PMS_CLASS "WebRtcService"

#include "Master/Defines.hpp"
#include "Master/WebRtcService.hpp"
#include "Master/Log.hpp"

namespace Master {

WebRtcService::WebRtcService(NetServer *messageServer)
{
    messageServer->SetListener(this);

    m_messageServer = messageServer;
}

WebRtcService::~WebRtcService()
{

}

void WebRtcService::SetMaster(MasterProcess *master) {
    m_master = master;
}

int WebRtcService::OnPlay(RtcSession *s, PlayValue *val)
{
    return this->Next()->OnPlay(s, val);
}

int WebRtcService::OnPublish(RtcSession *s, PublishValue *val)
{
    return this->Next()->OnPublish(s, val);
}

int WebRtcService::OnCloseStream(RtcSession *s, CloseStreamValue *val)
{
    return this->Next()->OnCloseStream(s, val);
}

int WebRtcService::OnMessage(NetRequest *request)
{
    PMS_DEBUG("OnMessage | uri {}, app {}, stream {}, method {} => {}",
        request->GetUri(), request->GetApp(), request->GetStreamName(),
        request->GetMethod(), request->GetData());

    return 0;
}

void WebRtcService::OnAborted(NetRequest *request)
{
//    PMS_DEBUG("OnAborted | {}", r);
    return;
}

}
