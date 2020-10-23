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

    pingos::Log::ClassInit(pingos::Configuration::log.file, pingos::Configuration::log.fileLevel, pingos::Configuration::log.fileLevel);
    pingos::Loop::ClassInit();
    pingos::Master::ClassInit(pingos::Loop::FetchLoop());

    pingos::WssServer ws;
    pingos::RtcMaster master;
    pingos::RtcServer rtc;

    if (pingos::Configuration::websocket.ssl) {
        ws.Accept(pingos::Configuration::websocket.port,
            pingos::Configuration::websocket.keyFile,
            pingos::Configuration::websocket.certFile,
            pingos::Configuration::websocket.passPhrase);
    } else {
        ws.Accept(pingos::Configuration::websocket.port);
    }

    rtc.Start(&ws, &master);

    master.Start();

    pingos::Loop::Run();
}
