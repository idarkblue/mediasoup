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

#define PMS_LOG_FILE "./logs/pms.log"

static std::string sdp = R"(
v=0
o=OnewaveUServerNG 1451516402 1025358037 IN IP4 192.168.20.136  
s=/xxx666  
u=http:///  
e=admin@  
c=IN IP4 0.0.0.0  
t=0 0  
a=isma-compliance:1,1.0,1  

a=range:npt=0-  
m=video 0 RTP/AVP 96
a=rtpmap:96 MP4V-ES/90000  
a=fmtp:96 profile-level-id=245;config=000001B0F5000001B509000001000000012000C888B0E0E0FA62D089028307
a=control:trackID=0
)";

void TestSdp()
{
    json jsonSdp = sdptransform::parse(sdp);
    std::cout << jsonSdp.dump() << std::endl;
}

int main(int argc, char **argv)
{
    TestSdp();

//    return 0;
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

    pingos::Loop::Run();

    return 0;
}
