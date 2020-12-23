#include <string>
#include <list>
#include "Master/Log.hpp"
#include "Master/RtcStream.hpp"

#define PMS_CLASS "pingos::RtcStream"

namespace pingos {

RtcStream::RtcStream(std::string streamId)
{
    this->streamId = streamId;
}

RtcStream::~RtcStream()
{

}

int RtcStream::Join(RtcSession *rtcSession)
{
    if (rtcSession ==nullptr) {
        PMS_ERROR("streamId[{}] join failed, rtcSession is null", this->streamId);

        return -1;
    }

    if (rtcSession->GetRole() == RtcSession::Role::PUBLISHER) {
        this->publisher = rtcSession;
    } else {
        this->playersMap[rtcSession->GetSessionId()] = rtcSession;
    }

    PMS_INFO("streamId[{}] join success, rtcSession[{}]", this->streamId, rtcSession->GetSessionId());

    return 0;
}

RtcSession* RtcStream::RemoveSession(std::string sessionId)
{
    RtcSession *rtcSession = nullptr;

    if (this->publisher && this->publisher->GetSessionId() == sessionId) {
        rtcSession = this->publisher;
        this->publisher = nullptr;
    } else if (this->playersMap.find(sessionId) != this->playersMap.end()) {
        rtcSession = this->playersMap[sessionId];
        this->playersMap.erase(sessionId);
    }

    return rtcSession;
}

RtcSession *RtcStream::GetPublisher()
{
    return this->publisher;
}

RtcSession *RtcStream::GetPlayer(std::string sid)
{
    if (this->playersMap.count(sid) == 0) {
        return nullptr;
    }

    return this->playersMap[sid];
}

} // namespace pingos
