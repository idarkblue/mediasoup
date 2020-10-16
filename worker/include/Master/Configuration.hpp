
#pragma once

#include <string>
#include <vector>
#include "Log.hpp"

namespace pingos {

struct LogConfiguration {
    std::string path;
    std::string level; // Log level, e.g. "info"
};

struct WebsocketConfiguration {
    uint16_t    port; // websocket port
    bool        ssl;
    std::string keyFile;
    std::string certFile;
    std::string passPhrase;
    std::string location; // uri, e.g. "/pingos"
};

struct WebRtcConfiguration {
    std::string listenIp; // local ip
    std::string announcedIp; // announced ip
    uint16_t minPort;
    uint16_t maxPort;
};

struct MasterConfiguration {
    int numOfWorkerProcess; // number of worker processes
    std::string execPath;
};

class Configuration
{
public:
    Configuration();
    virtual ~Configuration();

    static void ClassInit(std::string path);
    static int Load();

public:
    static LogConfiguration log;
    static WebsocketConfiguration websocket;
    static MasterConfiguration master;
    static WebRtcConfiguration webrtc;

private:
    static std::string m_path;
};

} // namespace
