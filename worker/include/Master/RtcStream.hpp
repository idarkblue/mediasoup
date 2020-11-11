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
    RtcSession* RemoveSession(std::string sessionId);

    RtcSession *GetPublisher();
    RtcSession *GetPlayer(std::string streamId);

private:
    std::string streamId { "" };
    RtcSession *publisher { nullptr };
    std::map<std::string, RtcSession *> playersMap;
};

} // namespace pingos

#endif
