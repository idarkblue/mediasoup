#pragma once
#include <string>
#include "json.hpp"
#include "RTC/RtpDictionaries.hpp"

using json = nlohmann::json;

namespace pingos {

class RtpParametersConstructor
{
public:
    RtpParametersConstructor() = default;
    RtpParametersConstructor(json &jsonData);
    virtual ~RtpParametersConstructor();

public:
    void FillJson(json& jsonData);

private:
    int TransformRtpParameters(json &jsonData);
    int TransformHeaderExtensions(json &jsonData, std::vector<RTC::RtpHeaderExtensionParameters> &headerExtensions);
    int TransformCodecs(json &jsonData, std::vector<RTC::RtpCodecParameters> &codecs);
    int TransformEncodings(json &jsonData, std::vector<RTC::RtpEncodingParameters> &encodings);
    int TransformRtcp(json &jsonData, RTC::RtcpParameters &rtcp);

public:
    RTC::RtpParameters rtpParameters;
};

}
