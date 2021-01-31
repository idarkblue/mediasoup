#pragma once

#include "json.hpp"
#include "RTC/PlainTransport.hpp"

using json = nlohmann::json;

namespace pingos {

class PlainTransportConstructor {
public:
    PlainTransportConstructor();
    virtual ~PlainTransportConstructor();

    int FillJson(json &jsonData);

public:
    uint16_t trackId { 0 };
    std::string listenIp { "" };
    std::string announcedIp { "" };
    bool comedia { true };
    bool rtcpMux { false };
    bool enableSrtp { false };
    std::string srtpCryptoSuite = { "" };
};

}
