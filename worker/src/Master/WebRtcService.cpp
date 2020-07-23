#define PMS_CLASS "WebRtcService"

#include "Master/Defines.hpp"
#include "Master/WebRtcService.hpp"
#include "Master/Log.hpp"

namespace Master {

WebRtcService::WebRtcService()
{

}

WebRtcService::~WebRtcService()
{

}

int WebRtcService::OnConnection(RtcSession *s, ConnectionValue *val)
{
    return this->Next()->OnConnection(s, val);
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

int WebRtcService::OnMessage(MasterRequest *r)
{
    PMS_DEBUG("OnMessage | {}", r->GetData());
    return 0;
}

void WebRtcService::OnAborted(MasterRequest *r)
{
    PMS_DEBUG("OnAborted | {}", (void *)r);
    return;
}

}
