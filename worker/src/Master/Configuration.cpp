#include <fstream>
#include <iostream>
#include "json.hpp"
#include "Master/Log.hpp"
#include "Master/Configuration.hpp"
#include "Master/Defines.hpp"
#include "MediaSoupErrors.hpp"

#define PMS_CLASS "pingos::Configuration"
#define MS_CLASS "pingos::Configuration"

using json = nlohmann::json;

namespace pingos{

LogConfiguration    Configuration::log;
WebsocketConfiguration Configuration::websocket;
MasterConfiguration Configuration::master;
WebRtcConfiguration Configuration::webrtc;
std::string Configuration::m_path;

Configuration::Configuration()
{

}

Configuration::~Configuration()
{

}

void Configuration::ClassInit(std::string path)
{
    m_path = path;
}

int Configuration::Load()
{
    std::string path = m_path;
    std::ifstream in(path, std::ios::binary);

    if (!in.is_open()){
        printf("Open Configuration file[%s] failed", path.c_str());
        return -1;
    }

    try {

        json jsonObject = json::parse(in);

        auto jsonLogIt = jsonObject.find("log");
        if (jsonLogIt == jsonObject.end()) {
            printf("Invalid configuration file, Missing log");
            return -1;
        }

        auto jsonWebsocketIt = jsonObject.find("websocket");
        if (jsonWebsocketIt == jsonObject.end()) {
            printf("Invalid configuration file, Missing websocket");
            return -1;
        }

        auto jsonMasterIt = jsonObject.find("master");
        if (jsonMasterIt == jsonObject.end()) {
            printf("Invalid configuration file, Missing master");
            return -1;
        }

        auto jsonWebRtcIt = jsonObject.find("webrtc");
        if (jsonWebRtcIt == jsonObject.end()) {
            printf("Invalid configuration file, Missing webrtc");
            return -1;
        }

        JSON_READ_VALUE_DEFAULT(*jsonLogIt, "file", std::string, log.file, "./logs/pms.log");
        JSON_READ_VALUE_DEFAULT(*jsonLogIt, "workerLevel", std::string, log.workerLevel, "warn");
        auto jsonTagsIt = jsonLogIt->find("workerTags");
        if (jsonTagsIt != jsonLogIt->end()) {
            for (auto &jsonTag : *jsonTagsIt) {
                log.workerTags.push_back(jsonTag.get<std::string>());
            }
        }
        JSON_READ_VALUE_DEFAULT(*jsonLogIt, "fileLevel", std::string, log.fileLevel, "info");
        JSON_READ_VALUE_DEFAULT(*jsonLogIt, "consolLevel", std::string, log.consolLevel, "info");

        JSON_READ_VALUE_DEFAULT(*jsonWebsocketIt, "port", uint16_t, websocket.port, 80);
        JSON_READ_VALUE_DEFAULT(*jsonWebsocketIt, "ssl", bool, websocket.ssl, false);
        JSON_READ_VALUE_DEFAULT(*jsonWebsocketIt, "keyFile", std::string, websocket.keyFile, "");
        JSON_READ_VALUE_DEFAULT(*jsonWebsocketIt, "certFile", std::string, websocket.certFile, "");
        JSON_READ_VALUE_DEFAULT(*jsonWebsocketIt, "passPhrase", std::string, websocket.passPhrase, "");
        JSON_READ_VALUE_DEFAULT(*jsonWebsocketIt, "location", std::string, websocket.location, "/");

        JSON_READ_VALUE_DEFAULT(*jsonWebRtcIt, "listenIp", std::string, webrtc.listenIp, "0.0.0.0");
        JSON_READ_VALUE_DEFAULT(*jsonWebRtcIt, "announcedIp", std::string, webrtc.announcedIp, "0.0.0.0");
        JSON_READ_VALUE_DEFAULT(*jsonWebRtcIt, "minPort", uint16_t, webrtc.minPort, 20000);
        JSON_READ_VALUE_DEFAULT(*jsonWebRtcIt, "maxPort", uint16_t, webrtc.maxPort, 30000);
        JSON_READ_VALUE_DEFAULT(*jsonWebRtcIt, "dtlsCertificateFile", std::string, webrtc.dtlsPrivateKeyFile, "");
        JSON_READ_VALUE_DEFAULT(*jsonWebRtcIt, "dtlsPrivateKeyFile", std::string, webrtc.dtlsPrivateKeyFile, "");

        JSON_READ_VALUE_DEFAULT(*jsonMasterIt, "numOfWorkerProcess", int, master.numOfWorkerProcess, 0);
        JSON_READ_VALUE_DEFAULT(*jsonMasterIt, "execPath", std::string, master.execPath, "./");
        if (master.execPath.empty()) {
            master.execPath = "./";
        }

        if (master.execPath.at(master.execPath.size() - 1) != '/') {
            master.execPath += "/";
        }

        JSON_READ_VALUE_DEFAULT(*jsonMasterIt, "unixSocketPath", std::string, master.unixSocketPath, "/tmp/pingos");
        JSON_READ_VALUE_DEFAULT(*jsonMasterIt, "workerName", std::string, master.workerName, "mediasoup-worker");

    } catch (const json::parse_error &error) {
        printf("Invalid configuration file, Parse Setting file[%s] failed, reason %s.",
            path.c_str(), error.what());
        in.close();

        return -1;
    }

    in.close();

    return 0;
}

}
