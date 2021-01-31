#include <fstream>
#include <iostream>
#include <string>
#include "Logger.hpp"
#include "Configuration.hpp"
#include "utils/JsonHelper.hpp"
#include "MediaSoupErrors.hpp"

#define MS_CLASS "pingos::Configuration"

using json = nlohmann::json;

namespace pingos{

LogConfiguration       Configuration::log;
WebsocketConfiguration Configuration::websocket;
HttpConfiguration      Configuration::http;
RtspConfiguration      Configuration::rtsp;
MasterConfiguration    Configuration::master;
WebRtcConfiguration    Configuration::webrtc;
RecordConfiguration    Configuration::record;
PullConfiguration      Configuration::pull;

std::string Configuration::filePath;

Configuration::Configuration()
{

}

Configuration::~Configuration()
{

}

void Configuration::ClassInit(std::string path)
{
    filePath = path;
    Load();
}

int Configuration::Load()
{
    std::string path = filePath;
    std::ifstream in(path, std::ios::binary);

    if (!in.is_open()){
        MS_THROW_ERROR("Open Configuration file[%s] failed\r\n", path.c_str());
        return -1;
    }

    try {

        json jsonObject = json::parse(in);

        auto jsonLogIt = jsonObject.find("log");
        if (jsonLogIt == jsonObject.end()) {
            MS_THROW_ERROR("Invalid configuration file, Missing log\r\n");
            return -1;
        }

        auto jsonWebsocketIt = jsonObject.find("websocket");
        if (jsonWebsocketIt == jsonObject.end()) {
            MS_THROW_ERROR("Invalid configuration file, Missing websocket\r\n");
            return -1;
        }

        auto jsonHttpIt = jsonObject.find("http");
        if (jsonHttpIt == jsonObject.end()) {
            MS_THROW_ERROR("Invalid configuration file, Missing http\r\n");
            return -1;
        }

        auto jsonRtspIt = jsonObject.find("rtsp");
        if (jsonRtspIt == jsonObject.end()) {
            MS_THROW_ERROR("Invalid configuration file, Missing rtsp\r\n");
            return -1;
        }

        auto jsonMasterIt = jsonObject.find("master");
        if (jsonMasterIt == jsonObject.end()) {
            MS_THROW_ERROR("Invalid configuration file, Missing master\r\n");
            return -1;
        }

        auto jsonWebRtcIt = jsonObject.find("webrtc");
        if (jsonWebRtcIt == jsonObject.end()) {
            MS_THROW_ERROR("Invalid configuration file, Missing webrtc\r\n");
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

        JSON_READ_VALUE_DEFAULT(*jsonWebsocketIt, "port", uint16_t, websocket.port, 0);
        JSON_READ_VALUE_DEFAULT(*jsonWebsocketIt, "sslPort", uint16_t, websocket.sslPort, 0);
        JSON_READ_VALUE_DEFAULT(*jsonWebsocketIt, "listenIp", std::string, websocket.listenIp, "0.0.0.0");
        JSON_READ_VALUE_DEFAULT(*jsonWebsocketIt, "keyFile", std::string, websocket.keyFile, "");
        JSON_READ_VALUE_DEFAULT(*jsonWebsocketIt, "certFile", std::string, websocket.certFile, "");
        JSON_READ_VALUE_DEFAULT(*jsonWebsocketIt, "passPhrase", std::string, websocket.passPhrase, "");
        JSON_READ_VALUE_DEFAULT(*jsonWebsocketIt, "location", std::string, websocket.location, "/");
        JSON_READ_VALUE_DEFAULT(*jsonWebsocketIt, "idleTimeout", int, websocket.idleTimeout, 100);
        JSON_READ_VALUE_DEFAULT(*jsonWebsocketIt, "maxBackpressure", int, websocket.maxBackpressure, 0);
        JSON_READ_VALUE_DEFAULT(*jsonWebsocketIt, "maxPayloadLength", int, websocket.maxPayloadLength, 16000000);

        JSON_READ_VALUE_DEFAULT(*jsonHttpIt, "port", uint16_t, http.port, 0);
        JSON_READ_VALUE_DEFAULT(*jsonHttpIt, "sslPort", uint16_t, http.sslPort, 0);
        JSON_READ_VALUE_DEFAULT(*jsonHttpIt, "listenIp", std::string, http.listenIp, "0.0.0.0");
        JSON_READ_VALUE_DEFAULT(*jsonHttpIt, "keyFile", std::string, http.keyFile, "");
        JSON_READ_VALUE_DEFAULT(*jsonHttpIt, "certFile", std::string, http.certFile, "");
        JSON_READ_VALUE_DEFAULT(*jsonHttpIt, "passPhrase", std::string, http.passPhrase, "");
        JSON_READ_VALUE_DEFAULT(*jsonHttpIt, "location", std::string, http.location, "/");

        JSON_READ_VALUE_DEFAULT(*jsonRtspIt, "port", uint16_t, rtsp.port, 0);
        JSON_READ_VALUE_DEFAULT(*jsonRtspIt, "listenIp", std::string, rtsp.listenIp, "0.0.0.0");

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

        JSON_READ_VALUE_DEFAULT(*jsonMasterIt, "workerName", std::string, master.workerName, "mediasoup-worker");

        auto jsonRecordIt = jsonObject.find("record");
        if (jsonRecordIt != jsonObject.end()) {
            JSON_READ_VALUE_DEFAULT(*jsonRecordIt, "targetHost", std::string, record.targetHost, "127.0.0.1");
            JSON_READ_VALUE_DEFAULT(*jsonRecordIt, "targetPort", uint16_t, record.targetPort, rtsp.port);
            JSON_READ_VALUE_DEFAULT(*jsonRecordIt, "recordPath", std::string, record.recordPath, "./record/");
            JSON_READ_VALUE_DEFAULT(*jsonRecordIt, "execRecordDone", std::string, record.execRecordDone, "");
            JSON_READ_VALUE_DEFAULT(*jsonRecordIt, "cmdPort", uint16_t, record.cmdPort, 8888);
        } else {
            record.targetHost = "127.0.0.1";
            record.targetPort = rtsp.port;
            record.recordPath = "./record/";
            record.execRecordDone = "";
            record.cmdPort = 8888;
        }

        if (record.recordPath.empty() || record.recordPath.at(record.recordPath.length() - 1) != '/') {
            record.recordPath += "/";
        }

        auto jsonPullIt = jsonObject.find("pull");
        if (jsonPullIt != jsonObject.end() && jsonPullIt->is_array()) {
            for (auto &jsonItem : *jsonPullIt) {
                PullConfiguration::ServerInfo si;
                JSON_READ_VALUE_ASSERT(jsonItem, std::string("ip"), std::string, si.ip);
                JSON_READ_VALUE_ASSERT(jsonItem, std::string("port"), uint16_t, si.port);
                pull.servers.push_back(si);
            }
        }

    } catch (const json::parse_error &error) {
        in.close();
        MS_THROW_ERROR("Invalid configuration file, Parse Setting file[%s] failed, reason %s.\r\n",
            path.c_str(), error.what());

        return -1;
    }

    in.close();

    return 0;
}

}
