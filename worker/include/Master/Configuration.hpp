
#pragma once

#include <string>
#include <vector>
#include "Log.hpp"

namespace pingos {

struct LogConfiguration {
    std::string path;
    std::string level; // Log level, e.g. "info"
    std::vector<std::string> tags; // e.g. ["info", "ice"]
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
    std::string dtlsCertificateFile;
    std::string dtlsPrivateKeyFile;
};

struct MasterConfiguration {
    int numOfWorkerProcess { 0 }; // number of worker processes
    std::string execPath = { "./" };
    std::string unixSocketPath { "/tmp/pingos" };
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
