#include <string>
#include <list>
#include "Master/Log.hpp"
#include "Master/RtcStream.hpp"

#define PMS_CLASS "pingos::RtcStream"

namespace pingos {

RtcStream::RtcStream(std::string streamId)
{

}

RtcStream::~RtcStream()
{

}

int RtcStream::Join(RtcSession *rtcSession)
{
    if (rtcSession ==nullptr) {
        PMS_ERROR("streamId[{}] join failed, rtcSession is null", m_streamId);

        return -1;
    }

    if (rtcSession->GetRole() == RtcSession::Role::PUBLISHER) {
        m_publisher = rtcSession;
    } else {
        m_playersMap[rtcSession->GetSessionId()] = rtcSession;
    }

    PMS_INFO("streamId[{}] join success, rtcSession[{}]", m_streamId, rtcSession->GetSessionId());

    return 0;
}

RtcSession* RtcStream::RemoveSession(std::string sessionId)
{
    RtcSession *rtcSession = nullptr;

    if (m_publisher && m_publisher->GetSessionId() == sessionId) {
        rtcSession = m_publisher;
        m_publisher = nullptr;
    } else if (m_playersMap.find(sessionId) != m_playersMap.end()) {
        rtcSession = m_playersMap[sessionId];
        m_playersMap.erase(sessionId);
    }

    return rtcSession;
}

RtcSession *RtcStream::GetPublisher()
{
    return m_publisher;
}

RtcSession *RtcStream::GetPlayer(std::string sid)
{
    if (m_playersMap.count(sid) == 0) {
        return nullptr;
    }

    return m_playersMap[sid];
}

} // namespace pingos
