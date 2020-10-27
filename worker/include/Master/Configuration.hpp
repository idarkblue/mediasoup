
#pragma once

#include <string>
#include <vector>
#include "Log.hpp"

namespace pingos {

struct LogConfiguration {
    std::string file;
    std::string workerLevel; // Log level, e.g. "debug" or "warn" or "error" or "none"
    std::vector<std::string> workerTags; // e.g. ["info", "ice"]
    std::string fileLevel; // Log level, e.g. "trace", "debug", "info", "warning", "error", "critical", "off"
    std::string consolLevel; // Log level, e.g. "trace", "debug", "info", "warning", "error", "critical", "off"
};

struct WebsocketConfiguration {
    uint16_t    port; // websocket port
    uint16_t    sslPort; // websocket ssl port
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
    std::string workerName = { "" };
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
