extern "C" {
    #include <stdio.h>
    #include <unistd.h>
}

#include <iostream>
#include "core/Loop.hpp"
#include "core/Configuration.hpp"
#include "core/Module.hpp"
#include "RtcMaster.hpp"
#include "network/HttpServer.hpp"
#include "network/WssServer.hpp"
#include "transform/sdptransform.hpp"
#include "logger/Log.hpp"

void Usage() {
    printf("Usage:\n");
    printf("\tOptDemo [-c config-file] [-r reload] [-s stop/restart]\n");
}

#define DEFAULT_CONFIG_FILE "./conf/pms.conf"


int MasterMain(int argc, char* argv[])
{
    std::string confile = "";
    int opt;
    const char *optstring = "c:r:s:";
    while ((opt = getopt(argc, argv, optstring)) != -1) {
        switch (opt) {
            case 'c':
            confile = optarg;
            break;

            case 'r':
            printf("ignore -r\r\n");
            break;

            case 's':
            printf("ignore -s\r\n");
            break;

            case '?':
            Usage();
            break;

            default:
            Usage();
            break;
        }
    }

    if (confile.empty()) {
        confile = DEFAULT_CONFIG_FILE;
    }

    pingos::Loop::ClassInit();
    pingos::Configuration::ClassInit(confile);

    pingos::Log::ClassInit(pingos::Configuration::log.file,
        pingos::Configuration::log.fileLevel, pingos::Configuration::log.fileLevel);
    pingos::MainProcess::ClassInit(pingos::Loop::FetchLoop());

    pingos::RtcMaster master;

    master.Start();

    pingos::Loop::Run();

    return 0;
}
