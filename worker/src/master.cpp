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

#define PMS_LOG_FILE "./logs/pms.log"

static const char *WorkerProcessArgs[] = {
    "--logLevel=debug",
    "--logTags=info",
    "--logTags=ice",
    "--logTags=rtp",
    nullptr
};

int main(int argc, char **argv)
{
    if (argc != 2) {
        printf("Configuration file must be specified.\r\n");
        return 0;
    }

    pingos::Log::ClassInit(PMS_LOG_FILE, spdlog::level::level_enum::trace, spdlog::level::level_enum::trace);
    pingos::Loop::ClassInit();
    pingos::Worker::ClassInit(WorkerProcessArgs, "./logs/pms.sock");
    pingos::Master::ClassInit(pingos::Loop::FetchLoop());
    pingos::Configuration::ClassInit(argv[1]);

    if (pingos::Configuration::Load() != 0) {
        printf("Configuration file load failed.\r\n");
        return 0;
    }

    pingos::WssServer ns;
    if (pingos::Configuration::websocket.ssl) {
        ns.Accept(pingos::Configuration::websocket.port,
            pingos::Configuration::websocket.keyFile,
            pingos::Configuration::websocket.certFile,
            pingos::Configuration::websocket.passPhrase);
    } else {
        ns.Accept(pingos::Configuration::websocket.port);
    }

    pingos::RtcMaster master;

    pingos::RtcServer rtc(&ns, &master);

    pingos::Master::Options opt = {
        .execDir = pingos::Configuration::master.execPath,
        .nWorkers = pingos::Configuration::master.numOfWorkerProcess,
        .daemon = false
    };

    master.StartWorkers(opt);

    pingos::Loop::Run();
}
