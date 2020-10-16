#ifndef _RTC_STREAM_HPP_INCLUDE_
#define _RTC_STREAM_HPP_INCLUDE_

#include <string>
#include <list>
#include "RtcSession.hpp"

namespace pingos {

class RtcStream {
public:
    RtcStream(std::string streamId);
    virtual ~RtcStream();

public:
    int Join(RtcSession *rtcSession);
    RtcSession* DeleteSession(std::string sessionId);

    RtcSession *GetPublisher();
    RtcSession *GetPlayer(std::string streamId);

private:
    // std::string m_server { "" };
    // std::string m_app { "" };
    std::string m_streamId { "" };
    RtcSession *m_publisher { nullptr };
    std::map<std::string, RtcSession *> m_playersMap;
};

} // namespace pingos

#endif
