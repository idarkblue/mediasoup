extern "C" {
    #include <stdio.h>
}
#include <string>
#include "Master/MasterProcess.hpp"
#include "Master/WorkerProcess.hpp"
#include "Master/HttpServer.hpp"
#include "Master/WebRtcService.hpp"
#include "Master/Log.hpp"
#include "Master/Loop.hpp"

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
    Master::Log::ClassInit(PMS_LOG_FILE, spdlog::level::level_enum::trace, spdlog::level::level_enum::trace);
    Master::Loop::ClassInit();
    Master::WorkerProcess::ClassInit(WorkerProcessArgs, "./logs/pms.sock");
    Master::MasterProcess::ClassInit(Master::Loop::FetchLoop());

    Master::HttpServer    http;
    Master::WebRtcService rtc(&http);
    Master::MasterProcess master;

    Master::MasterProcess::Options opt = {
        .execDir = "./",
        .nWorkers = 1,
        .daemon = false
    };

    master.ActiveWorkers(opt);
    master.RegisterFilter(&rtc);
    rtc.SetMaster(&master);
    http.StartHttp(8080);

    Master::Loop::Run();
}
