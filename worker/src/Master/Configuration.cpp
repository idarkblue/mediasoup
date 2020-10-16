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
        PMS_ERROR("Open Configuration file {} failed", path);
        return -1;
    }

    try {

        json jsonObject = json::parse(in);

        auto jsonLogIt = jsonObject.find("log");
        if (jsonLogIt == jsonObject.end()) {
            MS_THROW_ERROR("Missing log");
        }

        auto jsonWebsocketIt = jsonObject.find("websocket");
        if (jsonWebsocketIt == jsonObject.end()) {
            MS_THROW_ERROR("Missing websocket");
        }

        auto jsonMasterIt = jsonObject.find("master");
        if (jsonMasterIt == jsonObject.end()) {
            MS_THROW_ERROR("Missing master");
        }

        auto jsonWebRtcIt = jsonObject.find("webrtc");
        if (jsonWebRtcIt == jsonObject.end()) {
            MS_THROW_ERROR("Missing webrtc");
        }

        JSON_READ_VALUE_DEFAULT(*jsonLogIt, "path", std::string, log.path, "./logs/pms.log");
        JSON_READ_VALUE_DEFAULT(*jsonLogIt, "level", std::string, log.level, "info");

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

        JSON_READ_VALUE_DEFAULT(*jsonMasterIt, "numOfWorkerProcess", int, master.numOfWorkerProcess, 0);
        JSON_READ_VALUE_DEFAULT(*jsonMasterIt, "execPath", std::string, master.execPath, "");

    } catch (const json::parse_error &error) {
        PMS_ERROR("Parse Setting file[{}] failed, reason {}.", path, error.what());
        in.close();

        return -1;
    }

    in.close();

    return 0;
}

}
