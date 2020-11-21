#include "Worker/PlainTransportConstructor.hpp"

using json = nlohmann::json;

namespace pingos {

PlainTransportConstructor:: PlainTransportConstructor()
{
}

PlainTransportConstructor::~PlainTransportConstructor()
{

}

int PlainTransportConstructor::FillJson(json &jsonData)
{
    jsonData["listenIp"]["ip"] = this->listenIp;
    jsonData["listenIp"]["announcedIp"] = this->announcedIp;
    jsonData["rtcpMux"] = this->rtcpMux;
    jsonData["comedia"] = this->comedia;
    jsonData["enableSrtp"] = this->enableSrtp;
    if (this->enableSrtp) {
        jsonData["srtpCryptoSuite"] = this->srtpCryptoSuite;
    }

    return 0;
}

}
