extern "C" {
    #include <stdio.h>
}
#include <string>
#include "Master/Master.hpp"
#include "Master/Worker.hpp"
#include "Master/HttpServer.hpp"
#include "Master/WssServer.hpp"
#include "Master/RtcMaster.hpp"
#include "Master/Log.hpp"
#include "Master/Loop.hpp"
#include "Master/SdpInfo.hpp"
#include "Master/RtcServer.hpp"
#include "Master/Configuration.hpp"
#include "Master/sdptransform.hpp"
#include "Master/Rtsp/RtspServer.hpp"
#include "Master/Record/RecordServer.hpp"

#define PMS_LOG_FILE "./logs/pms.log"

int main(int argc, char **argv)
{
    if (argc != 2) {
        printf("Configuration file must be specified.\r\n");
        return 0;
    }

    pingos::Configuration::ClassInit(argv[1]);

    if (pingos::Configuration::Load() != 0) {
        printf("Configuration file load failed.\r\n");
        return 0;
    }

    pingos::Loop::ClassInit();
    pingos::Log::ClassInit(pingos::Configuration::log.file, pingos::Configuration::log.fileLevel, pingos::Configuration::log.fileLevel);
    pingos::Master::ClassInit(pingos::Loop::FetchLoop());

    pingos::RtcMaster master;

    if (master.Start() != 0) {
        printf("Failed to start master\r\n");
        return 0;
    }

    pingos::WssServer ws(
        uWS::CompressOptions::SHARED_COMPRESSOR,
        pingos::Configuration::websocket.maxPayloadLength,
        pingos::Configuration::websocket.idleTimeout,
        pingos::Configuration::websocket.maxBackpressure
    );

    if (pingos::Configuration::websocket.port) {
        if (ws.Accept(pingos::Configuration::websocket.listenIp,
            pingos::Configuration::websocket.port,
            pingos::Configuration::websocket.location) != 0)
        {
            return -1;
        }
    }

    if (!pingos::Configuration::websocket.certFile.empty() &&
        !pingos::Configuration::websocket.keyFile.empty() &&
        pingos::Configuration::websocket.sslPort)
    {
        if (ws.Accept(pingos::Configuration::websocket.listenIp,
            pingos::Configuration::websocket.sslPort,
            pingos::Configuration::websocket.location,
            pingos::Configuration::websocket.keyFile,
            pingos::Configuration::websocket.certFile,
            pingos::Configuration::websocket.passPhrase) != 0)
        {
            return -1;
        }
    }

    pingos::HttpServer http;

    if (pingos::Configuration::http.port) {
        if (http.Accept(pingos::Configuration::http.listenIp,
            pingos::Configuration::http.port,
            pingos::Configuration::http.location) != 0)
        {
            return -1;
        }
    }

    if (!pingos::Configuration::http.certFile.empty() &&
        !pingos::Configuration::http.keyFile.empty() &&
        pingos::Configuration::http.sslPort)
    {
        if (http.Accept(pingos::Configuration::http.listenIp,
            pingos::Configuration::http.sslPort,
            pingos::Configuration::http.location,
            pingos::Configuration::http.keyFile,
            pingos::Configuration::http.certFile,
            pingos::Configuration::http.passPhrase) != 0)
        {
            return -1;
        }
    }

    pingos::RtspServer rtspServer;
    pingos::TcpServer tcp(&rtspServer, &rtspServer,
        pingos::Configuration::rtsp.listenIp, pingos::Configuration::rtsp.port);

    rtspServer.SetMaster(&master);

    pingos::RtcServer rtcServer;

    rtcServer.SetMaster(&master);

    ws.SetListener(&rtcServer);
    http.SetListener(&rtcServer);

    pingos::HttpServer recordHttp;

    if (pingos::Configuration::record.cmdPort) {
        if (recordHttp.Accept("0.0.0.0", pingos::Configuration::record.cmdPort, "/") != 0) {
            return -1;
        }
    }

    pingos::RecordServer record(pingos::Loop::FetchLoop());
    recordHttp.SetListener(&record);

    pingos::Loop::Run();

    return 0;
}
