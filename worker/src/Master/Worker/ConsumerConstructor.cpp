#define PMS_CLASS "pingos::ConsumerConstructor"
#define MS_CLASS PMS_CLASS

#include "Log.hpp"
#include "Defines.hpp"
#include "sdptransform.hpp"
#include "Worker/ConsumerConstructor.hpp"

namespace pingos {

ConsumerConstructor::ConsumerConstructor(json &jsonData, RTC::RtpParameters::Type type, RTC::Media::Kind kind):
    rtpParameters(jsonData), type(type), kind(kind)
{
    this->TransformConsumer(jsonData);
}

ConsumerConstructor::~ConsumerConstructor()
{

}

int ConsumerConstructor::TransformConsumer(json &jsonData)
{
    JSON_READ_VALUE_THROW(jsonData, "direction", std::string, this->direction);
    this->paused = false;

    return 0;
}

int ConsumerConstructor::FillJson(std::vector<RTC::RtpEncodingParameters> &consumableRtpEncodings, json &jsonData)
{
    jsonData["kind"] = RTC::Media::GetString(this->kind);
    jsonData["type"] = RTC::RtpParameters::GetTypeString(this->type);
    jsonData["paused"] = this->paused;

    this->rtpParameters.FillJson(jsonData["rtpParameters"]);

    json jsonEncodings = json::array();
    for (auto &encoding : consumableRtpEncodings) {
        json jsonItem = json::object();
        jsonItem["ssrc"] = encoding.ssrc;
        jsonItem["active"] = true;
        jsonEncodings.emplace_back(jsonItem);
    }

    jsonData["consumableRtpEncodings"] = jsonEncodings;

    return 0;
}

}
