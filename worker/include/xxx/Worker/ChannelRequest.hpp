#pragma once

#include <string>
#include "json.hpp"

using json = nlohmann::json;

namespace pingos {

class ChannelRequest {
public:
    ChannelRequest() = default;
    ChannelRequest(const std::string method);
    virtual ~ChannelRequest();

    void Init(const std::string method);
    uint64_t GetId();
    std::string GetMethod();

public:
    void FillJson(json &jsonObject);

public:
    void SetInternal(json &jsonObject);
    void SetData(json &jsonObject);

private:
    uint64_t id { 0 };
    std::string method { "" };
    json jsonInternal;
    json jsonData;
    static uint64_t requestId;
};


}
