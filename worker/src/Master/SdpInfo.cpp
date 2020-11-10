#define PMS_CLASS "pingos::SdpInfo"

#include "Master/Log.hpp"
#include "Master/SdpInfo.hpp"
#include "Master/sdptransform.hpp"
#include "MediaSoupErrors.hpp"
#include "Master/Defines.hpp"

namespace pingos {

static uint32_t MAPPED_VIDEO_SSRC = 509921783;
static uint32_t MAPPED_AUDIO_SSRC = 994535925;

static std::unordered_set<RTC::RtpCodecMimeType::Subtype> SupportedCodec ( {
        RTC::RtpCodecMimeType::Subtype::VP8,
        RTC::RtpCodecMimeType::Subtype::OPUS
    } );

inline std::vector<std::string> splitOneOf(const std::string& str,
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

void WebRtcTransportParameters::AddLocalAddr(std::string ip, std::string announcedIp)
{
    WebRtcTransportParameters::LocalAddr addr;
    addr.ip = ip;
    addr.announcedIp = announcedIp;
    this->localAddrs.push_back(addr);
}

void WebRtcTransportParameters::AddFingerprint(Fingerprint &fingerprint)
{
    for (auto &it : dtlsParameters.fingerprints) {
        if (it.value == fingerprint.value) {
            return;
        }
    }

    dtlsParameters.fingerprints.push_back(fingerprint);
}

void WebRtcTransportParameters::FillJsonTransport(json &jsonObject)
{
    jsonObject = json::object();
    json jsonListenIps = json::array();

    for (auto &addr : this->localAddrs) {
        json jsonIp = json::object();
        jsonIp["ip"] = addr.ip;
        jsonIp["announcedIp"] = addr.announcedIp;

        jsonListenIps.emplace_back(jsonIp);
    }
    jsonObject["listenIps"] = jsonListenIps;
    jsonObject["enableUdp"] = this->enableUdp;
    jsonObject["enableTcp"] = this->enableTcp;
    jsonObject["enableSctp"] = this->enableSctp;
    jsonObject["preferUdp"] = this->preferUdp;
    jsonObject["preferTcp"] = this->preferTcp;
}

void WebRtcTransportParameters::FillJsonDtls(json &jsonObject)
{
    jsonObject = json::object();
    json jsonDtls = json::object();
    jsonDtls["role"] = "auto";

    json jsonFingerprints = json::array();
    for (auto &fingerprint : this->dtlsParameters.fingerprints) {
        json jsonFingerprint = json::object();
        jsonFingerprint["algorithm"] = fingerprint.algorithm;
        jsonFingerprint["value"] = fingerprint.value;
        jsonFingerprints.emplace_back(jsonFingerprint);
    }

    jsonDtls["fingerprints"] = jsonFingerprints;
    jsonObject["dtlsParameters"] = jsonDtls;
}

void ProducerParameters::FillJson(json &jsonObject)
{
    jsonObject = json::object();
    jsonObject["rtpParameters"] = json::object();
    jsonObject["kind"] = this->kind;
    jsonObject["paused"] = this->paused;

    this->rtpParameters.FillJson(jsonObject["rtpParameters"]);

    json jsonRtpMapping = json::object();
    json jsonCodecs = json::array();
    json jsonEncodings = json::array();

    for (auto it : this->rtpMapping.codecs) {
        json jsonCodec = json::object();
        jsonCodec["payloadType"] = it.first;
        jsonCodec["mappedPayloadType"] = it.second;
        jsonCodecs.emplace_back(jsonCodec);
    }
    jsonRtpMapping["codecs"] = jsonCodecs;

    for (auto &encoding : this->rtpMapping.encodings) {
        json jsonEncoding = json::object();
        if (encoding.ssrc) {
            jsonEncoding["ssrc"] = encoding.ssrc;
        }

        if (encoding.mappedSsrc) {
            jsonEncoding["mappedSsrc"] = encoding.mappedSsrc;
        }

        if (!encoding.rid.empty()) {
            jsonEncoding["rid"] = encoding.rid;
        }

        jsonEncodings.emplace_back(jsonEncoding);
    }
    jsonRtpMapping["encodings"] = jsonEncodings;

    jsonObject["rtpMapping"] = jsonRtpMapping;
}

int ConsumerParameters::SetRtpParameters(ProducerParameters &producer)
{
    this->type = "simple";
    this->kind = producer.kind;
    this->paused = producer.paused;

    this->rtpParameters.rtcp = producer.rtpParameters.rtcp;
    this->rtpParameters.hasRtcp = producer.rtpParameters.hasRtcp;
    this->rtpParameters.encodings = producer.rtpParameters.encodings;
    for (auto &encoding : this->rtpParameters.encodings) {
        encoding.hasCodecPayloadType = false;
    }

    this->consumableRtpEncodings = producer.rtpParameters.encodings;

    size_t i = 0;
    for (auto &cp : this->consumableRtpEncodings) {
        cp.rtx = RTC::RtpRtxParameters();
        cp.hasRtx = false;

        if (i >= producer.rtpMapping.encodings.size()) {
            return -1;
        }

        auto &encoding = producer.rtpMapping.encodings[i];
        cp.ssrc = encoding.mappedSsrc;
        i++;
    }

    return 0;
}

void ConsumerParameters::FillJson(json &jsonObject)
{
    jsonObject = json::object();
    jsonObject["kind"] = this->kind;
    jsonObject["type"] = this->type;
    jsonObject["paused"] = this->paused;

    this->rtpParameters.FillJson(jsonObject["rtpParameters"]);

    json jsonEncodings = json::array();
    for (auto &encoding : this->consumableRtpEncodings) {
        json jsonItem = json::object();
        jsonItem["ssrc"] = encoding.ssrc;
        jsonItem["active"] = true;
        jsonEncodings.emplace_back(jsonItem);
    }

    jsonObject["consumableRtpEncodings"] = jsonEncodings;
}

SdpInfo::SdpInfo(std::string sdp)
{
    m_sdp = sdp;
}

SdpInfo::~SdpInfo()
{

}

// Producer
int SdpInfo::TransformSdp(WebRtcTransportParameters &rtcTransportParameters,
    std::vector<ProducerParameters> &producerParameters)
{
    try {
        auto jsonSdp = sdptransform::parse(m_sdp);
        PMS_DEBUG("transform sdp : {}", jsonSdp.dump());
        if (ParseWebRtcTransport(jsonSdp, rtcTransportParameters) != 0) {
            PMS_ERROR("Parse webrtc transport failed");
            return -1;
        }

        if (ParseProducers(jsonSdp, producerParameters) != 0) {
            PMS_ERROR("Parse producers failed");
            return -1;
        }

        PMS_DEBUG("Parse producers success, producer number: {}", producerParameters.size());

    } catch (const json::parse_error &error) {
        PMS_ERROR("JSON parsing error.");
        return -1;
    } 

    return 0;
}

// Consumer
int SdpInfo::TransformSdp(WebRtcTransportParameters &rtcParameters, std::vector<ConsumerParameters> &consumerParameters)
{
    json jsonSdp;

    try {
        jsonSdp = sdptransform::parse(m_sdp);
    } catch (const json::parse_error &error) {
        PMS_ERROR("sdp transform failed");
        return -1;
    }

    if (ParseWebRtcTransport(jsonSdp, rtcParameters) != 0) {
        PMS_ERROR("Parse transport parameters failed");
        return -1;
    }

    auto jsonMediaIt = jsonSdp.find("media");
    if (jsonMediaIt == jsonSdp.end() && jsonMediaIt->is_array()) {
        PMS_ERROR("Invalid sdp, 'media' object needed");

        return -1;
    }

    for (auto &jsonRtp : *jsonMediaIt) {
        ConsumerParameters consumer;
        auto jsonTypeIt = jsonRtp.find("type");
        if (jsonTypeIt == jsonRtp.end()) {
            PMS_ERROR("Invalid sdp, 'media's type' item needed");

            return -1;
        }

        auto jsonDirectionIt = jsonRtp.find("direction");
        if (jsonDirectionIt == jsonRtp.end()) {
            PMS_ERROR("Invalid sdp, missing direction");
            return -1;
        }

        consumer.direction = jsonDirectionIt->get<std::string>();

        JSON_READ_VALUE_ASSERT(jsonRtp, "mid", std::string, consumer.rtpParameters.mid);

        auto jsonExtIt = jsonRtp.find("ext");
        if (jsonExtIt != jsonRtp.end() && jsonExtIt->is_array()) {
            if (ParseHeaderExtensions(*jsonExtIt, consumer.rtpParameters.headerExtensions) != 0) {
                PMS_ERROR("Invalid sdp, parse ext faile.");
                return -1;
            }
        }

        if (ParseCodecs(jsonRtp, consumer.rtpParameters.codecs) != 0) {
            PMS_ERROR("Invalid sdp, parse codecs failed");
            return -1;
        }

        consumer.kind = jsonTypeIt->get<std::string>();
        consumer.paused = false;

        consumerParameters.push_back(consumer);
    }

    return 0;
}

int SdpInfo::ParseWebRtcTransport(json &jsonSdp, WebRtcTransportParameters &rtcTransportParameters)
{
    auto jsonFingerprintIt = jsonSdp.find("fingerprint");

    WebRtcTransportParameters::Fingerprint fingerprint;
    if (jsonFingerprintIt != jsonSdp.end() &&
        this->ParseFingerprint(*jsonFingerprintIt, fingerprint) == 0)
    {
        rtcTransportParameters.AddFingerprint(fingerprint);
    }

    auto jsonMediaIt = jsonSdp.find("media");
    if (jsonMediaIt == jsonSdp.end() && jsonMediaIt->is_array()) {
        PMS_ERROR("Invalid sdp, 'media' object needed");

        return -1;
    }

    for (auto &jsonRtp : *jsonMediaIt) {
        auto jsonProtocolIt = jsonRtp.find("protocol");
        if (jsonProtocolIt == jsonRtp.end()) {
            PMS_ERROR("Invalid sdp, 'protocol' item needed");
            return -1;
        }

        auto protocol = jsonProtocolIt->get<std::string>();
        if (protocol.find("udp") != std::string::npos ||
            protocol.find("UDP") != std::string::npos)
        {
            rtcTransportParameters.enableUdp = true;
        }

        if (protocol.find("tcp") != std::string::npos ||
            protocol.find("TCP") != std::string::npos)
        {
            rtcTransportParameters.enableTcp = true;
        }

        jsonFingerprintIt = jsonRtp.find("fingerprint");

        if (jsonFingerprintIt != jsonRtp.end() &&
            this->ParseFingerprint(*jsonFingerprintIt, fingerprint) == 0)
        {
            rtcTransportParameters.AddFingerprint(fingerprint);
        }

        std::string setup;
        JSON_READ_VALUE_DEFAULT(jsonRtp, "setup", std::string, setup, "actpass");
        if (setup == "actpass") {
            rtcTransportParameters.dtlsParameters.role = RTC::DtlsTransport::Role::AUTO;
        } else if (setup == "active") {
            rtcTransportParameters.dtlsParameters.role = RTC::DtlsTransport::Role::CLIENT;
        } else if (setup == "passive") {
            rtcTransportParameters.dtlsParameters.role = RTC::DtlsTransport::Role::SERVER;
        }
    }

    auto jsonOriginIt = jsonSdp.find("origin");
    if (jsonOriginIt == jsonSdp.end()) {
        PMS_ERROR("Invalid sdp, missing origin");
        return -1;
    }

    JSON_READ_VALUE_ASSERT(jsonSdp["origin"], "sessionId", uint64_t, rtcTransportParameters.sessionId);
    JSON_READ_VALUE_ASSERT(jsonSdp["origin"], "username", std::string, rtcTransportParameters.uesrName);

    return 0;
}

int SdpInfo::ParseFingerprint(json &jsonFingerprint, WebRtcTransportParameters::Fingerprint &fingerprint)
{
    auto jsonHash = jsonFingerprint.find("hash");
    auto jsonType = jsonFingerprint.find("type");

    if (jsonHash == jsonFingerprint.end() || jsonType == jsonFingerprint.end()) {
        PMS_ERROR("Invalid sdp, fingerprint's hash and type needed");
        return -1;
    }

    fingerprint.algorithm = jsonType->get<std::string>();
    fingerprint.value = jsonHash->get<std::string>();

    return 0;
}

int SdpInfo::ParseProducers(json &jsonSdp, std::vector<ProducerParameters> &producerParameters)
{
    auto jsonMediaIt = jsonSdp.find("media");
    if (jsonMediaIt == jsonSdp.end() && jsonMediaIt->is_array()) {
        PMS_ERROR("Invalid sdp, 'media' object needed");

        return -1;
    }

    for (auto &jsonRtp : *jsonMediaIt) {
        ProducerParameters producer;
        auto jsonTypeIt = jsonRtp.find("type");
        if (jsonTypeIt == jsonRtp.end()) {
            PMS_ERROR("Invalid sdp, 'media's type' item needed");

            return -1;
        }

        auto jsonDirectionIt = jsonRtp.find("direction");
        if (jsonDirectionIt == jsonRtp.end()) {
            PMS_ERROR("Invalid sdp, missing direction");
            return -1;
        }

        producer.direction = jsonDirectionIt->get<std::string>();
        producer.kind = jsonTypeIt->get<std::string>();
        producer.paused = false;

        if (ParseRtpParameters(jsonRtp, producer.rtpParameters) != 0) {
            PMS_ERROR("Invalid sdp, parse rtp parameters failed");

            return -1;
        }

        if (ParseRtpMapping(producer) != 0) {
            PMS_ERROR("Invalid sdp, parse rtp mapping failed");

            return -1;
        }

        producerParameters.push_back(producer);
    }

    return 0;
}

int SdpInfo::ParseRtpParameters(json &jsonRtp, RTC::RtpParameters &rtpParameters)
{
    JSON_READ_VALUE_ASSERT(jsonRtp, "mid", std::string, rtpParameters.mid);

    auto jsonExtIt = jsonRtp.find("ext");
    if (jsonExtIt != jsonRtp.end() && jsonExtIt->is_array()) {
        if (ParseHeaderExtensions(*jsonExtIt, rtpParameters.headerExtensions) != 0) {
            PMS_ERROR("Invalid sdp, parse ext failed");
            return -1;
        }
    }

    if (ParseCodecs(jsonRtp, rtpParameters.codecs) != 0) {
        PMS_ERROR("Invalid sdp, parse codecs failed");
        return -1;
    }

    if (ParseEncodings(jsonRtp, rtpParameters.encodings) != 0) {
        PMS_ERROR("Invalid sdp, parse encodings failed");
        return -1;
    }

    if (ParseRtcp(jsonRtp, rtpParameters.rtcp) != 0) {
        PMS_ERROR("Invalid sdp, parse rtcp failed");
        return -1;
    }

    rtpParameters.hasRtcp = true;

    return 0;
}

int SdpInfo::ParseHeaderExtensions(json &jsonHeaderExtensions, std::vector<RTC::RtpHeaderExtensionParameters> &headerExtensions)
{
    for(auto& jsonExtension : jsonHeaderExtensions){

        RTC::RtpHeaderExtensionParameters ext;

        JSON_READ_VALUE_ASSERT(jsonExtension, "uri", std::string, ext.uri)
        JSON_READ_VALUE_ASSERT(jsonExtension, "value", uint8_t, ext.id)

        ext.type = RTC::RtpHeaderExtensionUri::GetType(ext.uri);

        if (ext.type == RTC::RtpHeaderExtensionUri::Type::UNKNOWN) {
            PMS_ERROR("Header extension not supported, {} {}", ext.uri, ext.id);
            continue;
        }

        headerExtensions.push_back(ext);
    }

    return 0;
}

int SdpInfo::ParseCodecs(json &jsonRtp, std::vector<RTC::RtpCodecParameters> &codecs)
{
    std::string rtpType { "" };

    JSON_READ_VALUE_ASSERT(jsonRtp, "type", std::string, rtpType);

    auto jsonCodecsIt = jsonRtp.find("rtp");

    if (jsonCodecsIt == jsonRtp.end()) {
        PMS_ERROR("Invalid Sdp, Missing rtp");
        return -1;
    }

    std::vector<RTC::RtpCodecParameters> allCodecs;
    std::vector<RTC::RtpCodecParameters> allRtx;

    for (auto& jsonCodec: *jsonCodecsIt) {
        try {
            std::string codecType { "" };
            RTC::RtpCodecParameters codec;

            std::string channels { "1" };
            JSON_READ_VALUE_ASSERT(jsonCodec, "payload", uint8_t, codec.payloadType);
            JSON_READ_VALUE_ASSERT(jsonCodec, "rate", uint32_t, codec.clockRate);
            JSON_READ_VALUE_DEFAULT(jsonCodec, "encoding", std::string, channels, "1");
            codec.channels = (uint8_t) std::stoul(channels);

            JSON_READ_VALUE_ASSERT(jsonCodec, "codec", std::string, codecType);

            std::string mimeType = rtpType + std::string("/") + codecType;
            codec.mimeType.SetMimeType(mimeType);

            auto jsonParameters = jsonRtp.find("fmtp");
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

            auto jsonRtcpFbIt = jsonRtp.find("rtcpFb");
            if (jsonRtcpFbIt != jsonRtp.end()) {
                for (auto &jsonRtcpFb : *jsonRtcpFbIt) {
                    if (codec.payloadType != (uint8_t) std::stoul(jsonRtcpFb["payload"].get<std::string>())) {
                        continue;
                    }
                    RTC::RtcpFeedback fb;
                    JSON_READ_VALUE_ASSERT(jsonRtcpFb, "type", std::string, fb.type);
                    JSON_READ_VALUE_DEFAULT(jsonRtcpFb, "subtype", std::string, fb.parameter, "");
                    codec.rtcpFeedback.push_back(fb);
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

        if (SupportedCodec.find(codec.mimeType.subtype) == SupportedCodec.end()) {
            continue;
        }

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

int SdpInfo::ParseEncodings(json &jsonRtp, std::vector<RTC::RtpEncodingParameters> &encodings)
{
    struct SsrcInfo {
        uint32_t ssrc { 0 };
        uint32_t rtxSsrc { 0 };
    };

    std::vector<SsrcInfo> ssrcInfos;

    auto jsonSsrcGroupsIt = jsonRtp.find("ssrcGroups");
    if (jsonSsrcGroupsIt != jsonRtp.end() && jsonSsrcGroupsIt->is_array()) {
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
    auto jsonSsrcsIt = jsonRtp.find("ssrcs");
    if (jsonSsrcsIt != jsonRtp.end()) {
        for (auto &jsonSsrc : *jsonSsrcsIt) {
            std::string attribute { "" };
            JSON_READ_VALUE_ASSERT(jsonSsrc, "attribute", std::string, attribute);
            if (attribute != "cname") {
                continue;
            }
            JSON_READ_VALUE_ASSERT(jsonSsrc, "id", uint32_t, ssrc);
            break;
        }
    }

    if (ssrcInfos.size() == 0) {
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

int SdpInfo::ParseRtcp(json &jsonRtp, RTC::RtcpParameters &rtcp)
{
    auto jsonSsrcsIt = jsonRtp.find("ssrcs");
    if (jsonSsrcsIt != jsonRtp.end()) {
        for (auto &jsonSsrc : *jsonSsrcsIt) {
            std::string attribute { "" };
            JSON_READ_VALUE_ASSERT(jsonSsrc, "attribute", std::string, attribute);
            if (attribute != "cname") {
                continue;
            }
            JSON_READ_VALUE_ASSERT(jsonSsrc, "value", std::string, rtcp.cname);
            break;
        }
    }

    return 0;
}

int SdpInfo::ParseRtpMapping(ProducerParameters &producer)
{
    auto &codecsMapping = producer.rtpMapping.codecs;
    auto &encodingsMapping = producer.rtpMapping.encodings;

    for (auto &codec : producer.rtpParameters.codecs) {
        codecsMapping[codec.payloadType] = codec.payloadType;
    }

    for (auto &encoding : producer.rtpParameters.encodings) {
        ProducerParameters::RtpEncodingMapping rtpEncoding;
        rtpEncoding.ssrc = encoding.ssrc;
        if (producer.kind == "audio") {
            rtpEncoding.mappedSsrc = MAPPED_AUDIO_SSRC;
        } else {
            rtpEncoding.mappedSsrc = MAPPED_VIDEO_SSRC;
        }
        encodingsMapping.push_back(rtpEncoding);
    }

    return 0;
}

}
