#pragma once

#include <string>
#include <iostream>
#include <unordered_map>
#include "UnixStreamSocket.hpp"
#include "Worker.hpp"
#include "Master/RtcStream.hpp"

extern "C" {
    #include <uv.h>
}

namespace pingos {

class RtcWorker : public Worker
{
public:
    RtcWorker(uv_loop_t *loop);
    virtual ~RtcWorker();

    int SendRequest(RtcSession *rtcSession, RtcSession::Request &request);

public: // Implement Worker
    virtual int ReceiveMasterMessage(std::string &payload) override;
    virtual void ReceiveChannelMessage(std::string_view &payload) override;

public:
    RtcSession *FindPublisher(std::string streamId);
    RtcSession *FindRtcSession(std::string streamId, std::string sessionId);
    RtcSession *CreateSession(std::string streamId, std::string sessionId, RtcSession::Role role);
    void DeleteSession(std::string streamId, std::string sessionId);

private:
    void ReceiveChannelAck(json &jsonObject);
    void ReceiveChannelEvent(json &jsonObject);

private:
    struct RequestWaittingInfo {
        std::string streamId;
        std::string sessionId;
    };

    std::unordered_map<uint64_t, RequestWaittingInfo> requestWaittingMap;

private:
    static std::unordered_map<std::string, RtcStream*> streamsMap;
};

}
