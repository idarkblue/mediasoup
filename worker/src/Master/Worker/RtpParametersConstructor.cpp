#define PMS_CLASS "pnigos::RtpParametersConstructor"
#define MS_CLASS PMS_CLASS

#include <string>
#include <unordered_set>
#include "Log.hpp"
#include "Defines.hpp"
#include "sdptransform.hpp"
#include "MediaSoupErrors.hpp"
#include "Worker/RtpParametersConstructor.hpp"

namespace pingos {

static inline std::vector<std::string> splitOneOf(const std::string& str,
                                    const std::string delims,
                                    const size_t maxSplits = 0) {
  std::string remaining(str);
  std::vector<std::string> result;
  size_t splits = 0, pos;

  while (((maxSplits == 0) || (splits < maxSplits)) &&
         ((pos = remaining.find_first_of(delims)) != std::string::npos)) {
    result.push_back(remaining.substr(0, pos));
    remaining = remaining.substr(pos + 1);
    splits++;
  }

  if (remaining.length() > 0)
    result.push_back(remaining);

  return result;
}

RtpParametersConstructor::RtpParametersConstructor(json &jsonData)
{
    this->TransformRtpParameters(jsonData);
}

RtpParametersConstructor::~RtpParametersConstructor()
{

}

void RtpParametersConstructor::FillJson(json& jsonData)
{
    this->rtpParameters.FillJson(jsonData);
}

int RtpParametersConstructor::TransformRtpParameters(json &jsonData)
{
    std::string mid;

    JSON_READ_VALUE_THROW(jsonData, "mid", std::string, mid);

    auto jsonExtIt = jsonData.find("ext");
    if (jsonExtIt != jsonData.end() && jsonExtIt->is_array()) {
        if (TransformHeaderExtensions(*jsonExtIt, this->rtpParameters.headerExtensions) != 0) {
            PMS_ERROR("Invalid sdp, parse ext failed");
            return -1;
        }
    }

    if (TransformCodecs(jsonData, this->rtpParameters.codecs) != 0) {
        PMS_ERROR("Invalid sdp, parse codecs failed");
        return -1;
    }

    if (TransformEncodings(jsonData, this->rtpParameters.encodings) != 0) {
        PMS_ERROR("Invalid sdp, parse encodings failed");
        return -1;
    }

    if (TransformRtcp(jsonData, this->rtpParameters.rtcp) != 0) {
        PMS_ERROR("Invalid sdp, parse rtcp failed");
        return -1;
    }

    this->rtpParameters.hasRtcp = this->rtpParameters.rtcp.ssrc != 0;
    this->rtpParameters.mid = mid;

    return 0;
}

int RtpParametersConstructor::TransformHeaderExtensions(json &jsonData, std::vector<RTC::RtpHeaderExtensionParameters> &headerExtensions)
{
    for(auto& jsonExtension : jsonData) {

        RTC::RtpHeaderExtensionParameters ext;

        JSON_READ_VALUE_THROW(jsonExtension, "uri", std::string, ext.uri)
        JSON_READ_VALUE_THROW(jsonExtension, "value", uint8_t, ext.id)

        json jsonExt;

        jsonExt["uri"] = ext.uri;
        jsonExt["id"] = ext.id;

        ext.type = RTC::RtpHeaderExtensionUri::GetType(ext.uri);

        if (ext.type == RTC::RtpHeaderExtensionUri::Type::UNKNOWN) {
            PMS_WARN("Header extension not supported, {} {}", ext.uri, ext.id);
            continue;
        }

        headerExtensions.emplace_back(jsonExt);
    }

    return 0;
}

int RtpParametersConstructor::TransformCodecs(json &jsonData, std::vector<RTC::RtpCodecParameters> &codecs)
{
    std::string rtpType { "" };

    auto jsonCodecsIt = jsonData.find("rtp");

    if (jsonCodecsIt == jsonData.end()) {
        PMS_ERROR("Invalid Sdp, Missing rtp");
        return -1;
    }

    std::vector<RTC::RtpCodecParameters> allCodecs;
    std::vector<RTC::RtpCodecParameters> allRtx;

    for (auto& jsonCodec: *jsonCodecsIt) {
        try {
            std::string codecType { "" };
            RTC::RtpCodecParameters codec;

            std::string channels;
            JSON_READ_VALUE_THROW(jsonCodec, "payload", uint8_t, codec.payloadType);
            JSON_READ_VALUE_THROW(jsonCodec, "rate", uint32_t, codec.clockRate);
            JSON_READ_VALUE_DEFAULT(jsonCodec, "encoding", std::string, channels, "1");
            codec.channels = (uint8_t) std::stoul(channels);

            JSON_READ_VALUE_THROW(jsonCodec, "codec", std::string, codecType);

            std::string mimeType = rtpType + std::string("/") + codecType;

            codec.mimeType.SetMimeType(mimeType);

            auto jsonParameters = jsonData.find("fmtp");
            for (auto &jsonFmtp : *jsonParameters) {
                auto jsonFmtpPayloadIt = jsonFmtp.find("payload");
                if (jsonFmtpPayloadIt == jsonFmtp.end() ||
                    codec.payloadType != jsonFmtpPayloadIt->get<uint8_t>())
                {
                    continue;
                }

                auto jsonFmtpConfigIt = jsonFmtp.find("config");
                if (jsonFmtpConfigIt == jsonFmtp.end()) {
                    continue;
                }

                std::string config = jsonFmtpConfigIt->get<std::string>();

                auto items = splitOneOf(config, ";");
                json obj = json::object();
                for (auto &item : items) {
                    auto kv = splitOneOf(item, "=");
                    if (kv.size() != 2) {
                        continue;
                    }
                    obj[kv[0]] = std::stoul(kv[1]);
                }
                codec.parameters.Set(obj);
            }

            auto jsonRtcpFbIt = jsonData.find("rtcpFb");
            if (jsonRtcpFbIt != jsonData.end()) {
                for (auto &jsonRtcpFb : *jsonRtcpFbIt) {
                    if (codec.payloadType != (uint8_t) std::stoul(jsonRtcpFb["payload"].get<std::string>())) {
                        continue;
                    }
                    RTC::RtcpFeedback fb;
                    JSON_READ_VALUE_THROW(jsonRtcpFb, "type", std::string, fb.type);
                    JSON_READ_VALUE_DEFAULT(jsonRtcpFb, "subtype", std::string, fb.parameter, "");

                    json jsonFb;
                    jsonFb["type"] = fb.type;
                    jsonFb["parameter"] = fb.parameter;
                    codec.rtcpFeedback.emplace_back(jsonFb);
                }
            }

            if (codecType == "rtx") {
                allRtx.push_back(codec);
            } else {
                allCodecs.push_back(codec);
            }
        } catch (const MediaSoupTypeError& error) {
            PMS_ERROR("catch error {}", error.what());
            continue;
        }
    }

    for (auto &codec : allCodecs) {

        codecs.push_back(codec);

        for (auto &rtx : allRtx) {
            if (!rtx.parameters.HasInteger("apt") ||
                rtx.parameters.GetInteger("apt") != codec.payloadType)
            {
                continue;
            }

            codecs.push_back(rtx);
            break;
        }

        break;
    }

    return 0;
}

int RtpParametersConstructor::TransformEncodings(json &jsonData, std::vector<RTC::RtpEncodingParameters> &encodings)
{
    struct SsrcInfo {
        uint32_t ssrc { 0 };
        uint32_t rtxSsrc { 0 };
    };

    std::vector<SsrcInfo> ssrcInfos;

    auto jsonSsrcGroupsIt = jsonData.find("ssrcGroups");
    if (jsonSsrcGroupsIt != jsonData.end() && jsonSsrcGroupsIt->is_array()) {
        for (auto &jsonSsrcGroup : *jsonSsrcGroupsIt) {
            auto jsonSsrcsIt = jsonSsrcGroup.find("ssrcs");
            if (jsonSsrcsIt == jsonSsrcGroup.end()) {
                continue;
            }

            std::string ssrcs = jsonSsrcsIt->get<std::string>();
            auto items = splitOneOf(ssrcs, " ");

            SsrcInfo ssrcInfo;
            ssrcInfo.ssrc = (uint32_t) std::stoul(items[0]);
            ssrcInfo.rtxSsrc = (uint32_t) std::stoul(items[1]);

            ssrcInfos.emplace_back(ssrcInfo);
            break;
        }
    }

    uint32_t ssrc { 0 };
    auto jsonSsrcsIt = jsonData.find("ssrcs");
    if (jsonSsrcsIt != jsonData.end()) {
        for (auto &jsonSsrc : *jsonSsrcsIt) {
            std::string attribute { "" };
            JSON_READ_VALUE_THROW(jsonSsrc, "attribute", std::string, attribute);
            if (attribute != "cname") {
                continue;
            }
            JSON_READ_VALUE_THROW(jsonSsrc, "id", uint32_t, ssrc);
            break;
        }
    }

    if (ssrcInfos.size() == 0 && ssrc != 0) {
        SsrcInfo ssrcInfo ;
        ssrcInfo.ssrc = ssrc;
        ssrcInfo.rtxSsrc = 0;

        ssrcInfos.emplace_back(ssrcInfo);
    }

    for (auto &ssrcInfo : ssrcInfos) {
        json jsonEncodings = json::object();
        jsonEncodings["active"] = true;
        if (ssrcInfo.ssrc > 0) {
            jsonEncodings["ssrc"] = ssrcInfo.ssrc;
        }

        if (ssrcInfo.rtxSsrc > 0) {
            json jsonRtx = json::object();
            jsonRtx["ssrc"] = ssrcInfo.rtxSsrc;
            jsonEncodings["rtx"] = jsonRtx;
        }

        RTC::RtpEncodingParameters rtpEncodingParameters(jsonEncodings);
        encodings.emplace_back(rtpEncodingParameters);
    }

    return 0;
}

int RtpParametersConstructor::TransformRtcp(json &jsonData, RTC::RtcpParameters &rtcp)
{
    auto jsonSsrcsIt = jsonData.find("ssrcs");
    if (jsonSsrcsIt != jsonData.end()) {
        for (auto &jsonSsrc : *jsonSsrcsIt) {
            std::string attribute { "" };
            JSON_READ_VALUE_THROW(jsonSsrc, "attribute", std::string, attribute);
            if (attribute != "cname") {
                continue;
            }
            JSON_READ_VALUE_ASSERT(jsonSsrc, "value", std::string, rtcp.cname);
            break;
        }
    }

    return 0;
}

}
