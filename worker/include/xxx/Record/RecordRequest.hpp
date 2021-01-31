#pragma once

#include <string>
#include "json.hpp"
#include "NetServer.hpp"

using json = nlohmann::json;

namespace pingos {
class RecordRequest {
public:
    enum class MethodId {
        INVALID_METHOD = 0,
        START_RECORD = 1,
        STOP_RECORD = 2
    };

    static std::unordered_map<std::string, MethodId> string2MethodId;

public:
    RecordRequest() = default;
    RecordRequest(NetConnection *netConnection);
    RecordRequest(NetConnection *nc, json &jsonObject);
    virtual ~RecordRequest();

    void Parse(json &jsonObject);

    static json& Parse(std::string message);

    int Accept();
    int Accept(json &jsonData);
    int Error(const char* reason = nullptr);

public:
    std::string version { "1.0" };
    std::string method { "" };
    std::string session { "" };
    std::string stream { "" };
    json jsonData;
    NetConnection *nc { nullptr };
    bool replied{ false };

    MethodId methodId { MethodId::INVALID_METHOD };
};

}
