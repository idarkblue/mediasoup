#pragma once

#include <string>
#include "json.hpp"
#include "NetServer.hpp"

using json = nlohmann::json;

namespace pingos {
class RtcRequest {
public:
    enum class MethodId {
        INVALID_METHOD = 0,
        SESSION_SETUP = 1,
        STREAM_PUBLISH = 2,
        STREAM_PLAY = 3,
        STREAM_MUTE = 4,
        STREAM_CLOSE = 5,
        STREAM_HEARTBEAT = 6,
        STREAM_SHUTDOWN = 7
    };

    static std::unordered_map<std::string, MethodId> string2MethodId;

public:
    RtcRequest() = default;
    RtcRequest(NetConnection *netConnection);
    RtcRequest(NetConnection *nc, json &jsonObject);
    virtual ~RtcRequest();

    void Parse(json &jsonObject);

    static json& Parse(std::string message);

    int Accept();
    int Accept(json &jsonData);
    int Error(const char* reason = nullptr);

public:
    int count { 0 };
    std::string version { "1.0" };
    std::string method { "" };
    std::string session { "" };
    std::string stream { "" };
    json jsonData;
    NetConnection *nc { nullptr };
    bool replied{ false };

    MethodId methodId { MethodId::INVALID_METHOD };
};

class RtcResponse {
public:
    RtcResponse(NetConnection *nc, std::string sessionId, std::string streamId, std::string method);
    virtual ~RtcResponse();

public:
    int Reply(int error, const char* reason, json &jsonObject);

public:
    std::string version { "1.0" };
    std::string method { "" };
    std::string session { "" };
    std::string stream { "" };

    NetConnection *nc { nullptr };
};

}
