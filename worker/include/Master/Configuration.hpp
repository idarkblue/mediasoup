
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
    std::string listenIp;
    std::string keyFile;
    std::string certFile;
    std::string passPhrase;
    std::string location; // uri, e.g. "/pingos"
    int idleTimeout;
    int maxBackpressure;
    int maxPayloadLength;
};

struct HttpConfiguration {
    uint16_t    port; // http port
    uint16_t    sslPort; // https port
    std::string listenIp;
    std::string keyFile;
    std::string certFile;
    std::string passPhrase;
    std::string location; // uri, e.g. "/pingos"
};

struct RtspConfiguration {
    uint16_t    port; // rtsp port
    std::string listenIp;
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
    static HttpConfiguration http;
    static RtspConfiguration rtsp;
    static MasterConfiguration master;
    static WebRtcConfiguration webrtc;

private:
    static std::string m_path;
};

} // namespace
