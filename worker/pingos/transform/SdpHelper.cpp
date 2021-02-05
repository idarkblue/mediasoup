#define MS_CLASS "pingos::SdpJson"
#include "utils/JsonHelper.hpp"
#include "core/Configuration.hpp"
#include "SdpHelper.hpp"
#include "Logger.hpp"
#include "MediaSoupErrors.hpp"
#include "transform/sdptransform.hpp"
#include "utils/StringHelper.hpp"

namespace pingos
{
    static std::map<std::string, std::string> sdp2DtlsRole = {
        { "actpass", "auto" },
        { "active", "client" },
        { "passive", "server" }
    };

    static std::map<std::string, std::string> dtls2SdpRole = {
        { "auto", "actpass" },
        { "client", "active" },
        { "server", "passive" }
    };

    #define DEFAULT_SDP_MSLABEL "lmy-cc-pingos"
    #define DEFAULT_SDP_USERNAME "lmy"
    #define DEFAULT_RTP_RTCP_CNAME "PINGOS"

    #define DEFAULT_AUDIO_RTP_SSRC 1017
    #define DEFAULT_VIDEO_RTP_SSRC 1124

    #define DEFAULT_AUDIO_RTX_SSRC 1990
    #define DEFAULT_VIDEO_RTX_SSRC 1989

    static std::map<RTC::Media::Kind, uint64_t> mappedSsrc = {
        { RTC::Media::Kind::AUDIO, DEFAULT_AUDIO_RTP_SSRC },
        { RTC::Media::Kind::VIDEO, DEFAULT_VIDEO_RTP_SSRC }
    };

    static std::map<RTC::Media::Kind, uint64_t> mappedRtxSsrc = {
        { RTC::Media::Kind::AUDIO, DEFAULT_AUDIO_RTX_SSRC },
        { RTC::Media::Kind::VIDEO, DEFAULT_VIDEO_RTX_SSRC }
    };

    int SdpHelper::ParseWebRtcConnect(std::string &sdp, std::vector<WebRtcConnect> &parameters)
    {
        json jsonSdp = sdptransform::parse(sdp);

        json jsonMedias;
        JSON_THROW_READ_ARRAY(jsonSdp, "media", jsonMedias);
        for (auto &jsonMedia : jsonMedias) {
            WebRtcConnect parameter;
            // dtls
            std::string setup;
            json jsonFingerprint;
            JSON_THROW_READ_OBJECT(jsonMedia, "fingerprint", jsonFingerprint);
            WebRtcConnect::DtlsParameters::Fingerprint fingerprint;
            JSON_THROW_READ_VALUE(jsonFingerprint, "type", std::string, string, fingerprint.algorithm);
            JSON_THROW_READ_VALUE(jsonFingerprint, "hash", std::string, string, fingerprint.value);
            parameter.dtls.fingerprints.push_back(fingerprint);

            JSON_THROW_READ_VALUE(jsonMedia, "setup", std::string, string, setup);
            if (sdp2DtlsRole.find(setup) == sdp2DtlsRole.end()) {
                MS_THROW_ERROR("Unknown setup[%s]", setup.c_str());
            }
            parameter.dtls.role = sdp2DtlsRole[setup];

            // ice
            JSON_THROW_READ_VALUE(jsonMedia, "icePwd", std::string, string, parameter.ice.pwd);
            JSON_THROW_READ_VALUE(jsonMedia, "iceUfrag", std::string, string, parameter.ice.ufrag);
            JSON_THROW_READ_VALUE(jsonMedia, "iceOptions", std::string, string, parameter.ice.options);

            // candidates
            auto jsonCandidatesIt = jsonMedia.find("candidates");
            if (jsonCandidatesIt != jsonMedia.end() && jsonCandidatesIt->is_array()) {

                json &jsonCandidates = *jsonCandidatesIt;
                for (auto &jsonCandidate : jsonCandidates) {
                    WebRtcConnect::Candidate candidate;
                    JSON_THROW_READ_VALUE(jsonCandidate, "foundation", std::string, string, candidate.foundation);
                    JSON_THROW_READ_VALUE(jsonCandidate, "ip", std::string, string, candidate.ip);
                    JSON_THROW_READ_VALUE(jsonCandidate, "port", uint16_t, number, candidate.port);
                    JSON_THROW_READ_VALUE(jsonCandidate, "priority", uint32_t, number, candidate.priority);
                    JSON_THROW_READ_VALUE(jsonCandidate, "transport", std::string, string, candidate.protocol);
                    JSON_THROW_READ_VALUE(jsonCandidate, "type", std::string, string, candidate.type);
                    JSON_THROW_READ_VALUE(jsonCandidate, "component", uint32_t, number, candidate.component);
                    parameter.candidates.push_back(candidate);
                }
            }

            JSON_THROW_READ_VALUE(jsonMedia, "mid", std::string, string, parameter.mid);

            parameters.push_back(parameter);
        }

        return 0;
    }

    int SdpHelper::ParseWebRtcTransport(json &jsonObject, WebRtcConnect &conn)
    {
        // ice
        std::string iceRole;
        JSON_THROW_READ_VALUE(jsonObject, "iceRole", std::string, string, conn.ice.iceRole);

        json jsonIceParameters;
        JSON_THROW_READ_OBJECT(jsonObject, "iceParameters", jsonIceParameters);
        JSON_THROW_READ_VALUE(jsonIceParameters, "usernameFragment", std::string, string, conn.ice.ufrag);
        JSON_THROW_READ_VALUE(jsonIceParameters, "password", std::string, string, conn.ice.pwd);
        JSON_THROW_READ_VALUE(jsonIceParameters, "iceLite", bool, boolean, conn.ice.iceLite);

        // dtls
        json jsonDtlsParameters;
        json jsonFingerprints;
        JSON_THROW_READ_OBJECT(jsonObject, "dtlsParameters", jsonDtlsParameters);
        JSON_THROW_READ_ARRAY(jsonDtlsParameters, "fingerprints", jsonFingerprints);
        for (auto &jsonFingerprint : jsonFingerprints) {
            WebRtcConnect::DtlsParameters::Fingerprint fingerprint;
            JSON_THROW_READ_VALUE(jsonFingerprint, "algorithm", std::string, string, fingerprint.algorithm);
            JSON_THROW_READ_VALUE(jsonFingerprint, "value", std::string, string, fingerprint.value);
            conn.dtls.fingerprints.push_back(fingerprint);
        }

        // candidate
        json jsonIceCandidates;
        JSON_THROW_READ_ARRAY(jsonObject, "iceCandidates", jsonIceCandidates);
        for (auto &jsonIceCandidate : jsonIceCandidates) {
            conn.candidates.emplace_back();
            auto &candidate = conn.candidates[conn.candidates.size() - 1];
            JSON_THROW_READ_VALUE(jsonIceCandidate, "foundation", std::string, string, candidate.foundation);
            JSON_THROW_READ_VALUE(jsonIceCandidate, "priority", uint32_t, number, candidate.priority);
            JSON_THROW_READ_VALUE(jsonIceCandidate, "ip", std::string, string, candidate.ip);
            JSON_THROW_READ_VALUE(jsonIceCandidate, "protocol", std::string, string, candidate.protocol);
            JSON_THROW_READ_VALUE(jsonIceCandidate, "port", uint16_t, number, candidate.port);
            JSON_THROW_READ_VALUE(jsonIceCandidate, "type", std::string, string, candidate.type);
            if (candidate.protocol == "tcp") {
                JSON_THROW_READ_VALUE(jsonIceCandidate, "tcpType", std::string, string, candidate.tcpType);
            }
        }

        return 0;
    }

    int SdpHelper::ParseTracks(std::string &sdp, std::vector<Track> &tracks)
    {
        json jsonSdp = sdptransform::parse(sdp);

        json jsonMedias;
        JSON_THROW_READ_ARRAY(jsonSdp, "media", jsonMedias);

        tracks.reserve(jsonMedias.size());
        for (auto &jsonMedia : jsonMedias) {
            tracks.emplace_back();
            Track &track = tracks[tracks.size() - 1];
            RTC::RtpParameters &rtpParameters = track.rtpParameters;

            std::string type, direction, mid;
            JSON_THROW_READ_VALUE(jsonMedia, "type", std::string, string, type);
            JSON_THROW_READ_VALUE(jsonMedia, "direction", std::string, string, direction);
            JSON_THROW_READ_VALUE(jsonMedia, "mid", std::string, string, mid);

            // header externsions
            auto jsonExtIt = jsonMedia.find("ext");
            if (jsonExtIt != jsonMedia.end()) {
                if (ParseHeaderExtensions(*jsonExtIt, rtpParameters.headerExtensions) != 0) {
                    MS_ERROR("Invalid sdp, parse ext failed");
                    return -1;
                }
            }

            if (ParseCodecs(jsonMedia, rtpParameters.codecs) != 0) {
                MS_ERROR("Invalid sdp, parse codecs failed");
                return -1;
            }

            if (ParseEncodings(jsonMedia, rtpParameters.encodings) != 0) {
                MS_ERROR("Invalid sdp, parse encodings failed");
                return -1;
            }

            if (ParseRtcp(jsonMedia, rtpParameters.rtcp) != 0) {
                rtpParameters.hasRtcp = false;
            } else {
                rtpParameters.hasRtcp = true;
            }

            std::string trackInfo, trackId;
            JSON_READ_VALUE_DEFAULT(jsonMedia, "control", std::string, trackInfo, "");
            auto kv = SplitOneOf(trackInfo, "=");
            if (kv.size() == 2) {
                trackId = kv[1];
            }

            rtpParameters.mid = mid;
            track.direction   = direction;
            track.kind        = RTC::Media::GetKind(type);
            track.trackId     = trackId;
        }

        return 0;
    }

    int SdpHelper::ParseHeaderExtensions(json &jsonHeaderExtensions, std::vector<RTC::RtpHeaderExtensionParameters> &headerExtensions)
    {
        for(auto& jsonExtension : jsonHeaderExtensions){

            RTC::RtpHeaderExtensionParameters ext;

            JSON_THROW_READ_VALUE(jsonExtension, "uri", std::string, string, ext.uri);
            JSON_THROW_READ_VALUE(jsonExtension, "value", uint8_t, number, ext.id);

            json jsonExt;

            jsonExt["uri"] = ext.uri;
            jsonExt["id"] = ext.id;

            ext.type = RTC::RtpHeaderExtensionUri::GetType(ext.uri);

            if (ext.type == RTC::RtpHeaderExtensionUri::Type::UNKNOWN) {
                MS_ERROR("Header extension not supported, %s %d", ext.uri.c_str(), ext.id);
                continue;
            }

            headerExtensions.emplace_back(jsonExt);
        }

        return 0;
    }

    int SdpHelper::ParseCodecs(json &jsonMedia, std::vector<RTC::RtpCodecParameters> &codecs)
    {
        std::string rtpType { "" };

        JSON_THROW_READ_VALUE(jsonMedia, "type", std::string, string, rtpType);

        auto jsonCodecsIt = jsonMedia.find("rtp");

        if (jsonCodecsIt == jsonMedia.end()) {
            MS_THROW_ERROR("Invalid Sdp, Missing rtp");
            return -1;
        }

        std::vector<RTC::RtpCodecParameters> allCodecs;
        std::vector<RTC::RtpCodecParameters> allRtx;

        for (auto& jsonCodec: *jsonCodecsIt) {
            try {
                std::string codecType { "" };
                RTC::RtpCodecParameters codec;

                std::string channels { "1" };
                JSON_THROW_READ_VALUE(jsonCodec, "payload", uint8_t, number, codec.payloadType);
                JSON_THROW_READ_VALUE(jsonCodec, "rate", uint32_t, number, codec.clockRate);
                JSON_DEFAULT_READ_VALUE(jsonCodec, "encoding", std::string, string, channels, "1");
                codec.channels = (uint8_t) std::stoul(channels);

                JSON_THROW_READ_VALUE(jsonCodec, "codec", std::string, string, codecType);

                std::string mimeType = rtpType + std::string("/") + codecType;
                codec.mimeType.SetMimeType(mimeType);

                auto jsonParameters = jsonMedia.find("fmtp");
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

                    auto items = SplitOneOf(config, ";");
                    json obj = json::object();
                    for (auto &item : items) {
                        auto kv = SplitOneOf(item, "=");
                        if (kv.size() != 2) {
                            continue;
                        }
                        obj[kv[0]] = std::stoul(kv[1]);
                    }
                    codec.parameters.Set(obj);
                }

                auto jsonRtcpFbIt = jsonMedia.find("rtcpFb");
                if (jsonRtcpFbIt != jsonMedia.end()) {
                    for (auto &jsonRtcpFb : *jsonRtcpFbIt) {
                        if (codec.payloadType != (uint8_t) std::stoul(jsonRtcpFb["payload"].get<std::string>())) {
                            continue;
                        }
                        RTC::RtcpFeedback fb;
                        JSON_THROW_READ_VALUE(jsonRtcpFb, "type", std::string, string, fb.type);
                        JSON_DEFAULT_READ_VALUE(jsonRtcpFb, "subtype", std::string, string, fb.parameter, "");

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
                MS_ERROR("catch error %s", error.what());
                continue;
            }
        }

        for (auto &codec : allCodecs) {

            // if (SupportedCodec.find(codec.mimeType.subtype) == SupportedCodec.end()) {
            //     continue;
            // }

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

    int SdpHelper::ParseEncodings(json &jsonMedia, std::vector<RTC::RtpEncodingParameters> &encodings)
    {
        struct SsrcInfo {
            uint32_t ssrc { 0 };
            uint32_t rtxSsrc { 0 };
        };

        std::vector<SsrcInfo> ssrcInfos;

        auto jsonSsrcGroupsIt = jsonMedia.find("ssrcGroups");
        if (jsonSsrcGroupsIt != jsonMedia.end() && jsonSsrcGroupsIt->is_array()) {
            for (auto &jsonSsrcGroup : *jsonSsrcGroupsIt) {
                auto jsonSsrcsIt = jsonSsrcGroup.find("ssrcs");
                if (jsonSsrcsIt == jsonSsrcGroup.end()) {
                    continue;
                }

                std::string ssrcs = jsonSsrcsIt->get<std::string>();
                auto items = SplitOneOf(ssrcs, " ");

                SsrcInfo ssrcInfo;
                if (items.size() == 1) {
                    ssrcInfo.ssrc = (uint32_t) std::stoul(items[0]);
                } else if (items.size() == 2) {
                    ssrcInfo.ssrc = (uint32_t) std::stoul(items[0]);
                    ssrcInfo.rtxSsrc = (uint32_t) std::stoul(items[1]);
                }

                ssrcInfos.emplace_back(ssrcInfo);
                break;
            }
        }

        uint32_t ssrc { 0 };
        auto jsonSsrcsIt = jsonMedia.find("ssrcs");
        if (jsonSsrcsIt != jsonMedia.end()) {
            for (auto &jsonSsrc : *jsonSsrcsIt) {
                std::string attribute { "" };
                JSON_THROW_READ_VALUE(jsonSsrc, "attribute", std::string, string, attribute);
                if (attribute != "cname") {
                    continue;
                }
                JSON_THROW_READ_VALUE(jsonSsrc, "id", uint32_t, number, ssrc);
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

            encodings.emplace_back(jsonEncodings);
        }

        return 0;
    }

    int SdpHelper::ParseRtcp(json &jsonMedia, RTC::RtcpParameters &rtcp)
    {
        auto jsonSsrcsIt = jsonMedia.find("ssrcs");
        if (jsonSsrcsIt != jsonMedia.end()) {
            for (auto &jsonSsrc : *jsonSsrcsIt) {
                std::string attribute { "" };
                JSON_THROW_READ_VALUE(jsonSsrc, "attribute", std::string, string, attribute);
                if (attribute != "cname") {
                    continue;
                }
                JSON_THROW_READ_VALUE(jsonSsrc, "value", std::string, string, rtcp.cname);
                break;
            }
        }

        if (rtcp.cname.empty()) {
            return -1;
        }

        return 0;
    }

    void SdpHelper::AutoFillEncodings(Track &track)
    {
        int32_t payloadType = -1;
        int32_t rtxPayloadType = -1;

        // search first supported codec
        for (auto &codec : track.rtpParameters.codecs) {
            if (!codec.mimeType.IsMediaCodec()) {
                continue;
            }
            payloadType = codec.payloadType;

            break;
        }

        // search rtx
        std::string aptKeyString = "apt";
        for (auto &codec : track.rtpParameters.codecs) {
            if (!codec.mimeType.IsFeatureCodec() ||
                codec.mimeType.subtype != RTC::RtpCodecMimeType::Subtype::RTX ||
                !codec.parameters.HasInteger(aptKeyString) ||
                codec.parameters.GetInteger(aptKeyString) != payloadType)
            {
                continue;
            }
            rtxPayloadType = codec.payloadType;

            break;
        }

        if (payloadType == -1) {
            MS_THROW_ERROR("No effect codec");
        }

        json jsonEncoding;
        jsonEncoding["ssrc"] = mappedSsrc[track.kind];
        if (rtxPayloadType != -1) {
            jsonEncoding["rtx"]["ssrc"] = mappedRtxSsrc[track.kind];
        }

        json jsonEncodings = json::array();
        jsonEncodings.push_back(jsonEncoding);

        track.rtpParameters.encodings.emplace_back(jsonEncodings);
    }

    void SdpHelper::AutoFillRtcp(Track &track)
    {
        json jsonRtcp;
        jsonRtcp["cname"] = DEFAULT_RTP_RTCP_CNAME;
        // jsonRtcp["reducedSize"] = true;
        // jsonSsrc["ssrc"] = mappedSsrc[track.kind];

        track.rtpParameters.hasRtcp = true;
    }

    void SdpHelper::ReverseTrack(Track &track)
    {
        if (track.direction == "sendonly") {
            track.direction = "recvonly";
        } else if (track.direction == "recvonly") {
            track.direction = "sendonly";
        }
    }

    int SdpHelper::Transform2WebRtcTransportJson(WebRtcConnect &parameter, json &jsonObject)
    {
        // dtls
        jsonObject["dtlsParameters"]["role"] = parameter.dtls.role;
        jsonObject["dtlsParameters"]["fingerprints"] = json::array();
        json jsonFingerprint;
        jsonFingerprint["algorithm"] = parameter.dtls.fingerprints[0].algorithm;
        jsonFingerprint["value"] = parameter.dtls.fingerprints[0].value;
        jsonObject["dtlsParameters"]["fingerprints"].push_back(jsonFingerprint);

        return 0;
    }

    int SdpHelper::Transform2ProducerJson(Track &parameter, json &jsonObject)
    {
        // rtpParameters
        parameter.rtpParameters.FillJson(jsonObject["rtpParameters"]);

        // rtpMapping
        json jsonCodecs, jsonEncodings;
        for (auto &codec : parameter.rtpParameters.codecs) {
            json jsonCodec;
            jsonCodec["payloadType"] = codec.payloadType;
            jsonCodec["mappedPayloadType"] = codec.payloadType;
        }

        for (auto &encoding : parameter.rtpParameters.encodings) {
            json jsonEncoding;
            jsonEncoding["ssrc"] = encoding.ssrc;
            jsonEncoding["mappedSsrc"] = mappedSsrc[parameter.kind];
        }

        jsonObject["rtpMapping"]["codecs"] = jsonCodecs;
        jsonObject["rtpMapping"]["encodings"] = jsonEncodings;

        // kind
        jsonObject["kind"] = RTC::Media::GetString(parameter.kind);

        // paused
        jsonObject["paused"] = false;

        return 0;
    }

    int SdpHelper::Transform2SimpleConsumerJson(Track &parameter, json &jsonObject)
    {
        // rtpParameters
        parameter.rtpParameters.FillJson(jsonObject["rtpParameters"]);

        // consumableRtpEncodings
        json consumableRtpEncoding;
        consumableRtpEncoding["ssrc"] = mappedSsrc[parameter.kind];

        jsonObject["consumableRtpEncodings"] = json::array();
        jsonObject["consumableRtpEncodings"].push_back(consumableRtpEncoding);

        // type
        jsonObject["type"] = "simple";

        // kind
        jsonObject["kind"] = RTC::Media::GetString(parameter.kind);

        // paused
        jsonObject["paused"] = false;

        return 0;
    }

    int SdpHelper::Transform2Sdp(std::vector<WebRtcConnect> &transports, std::vector<Track> &tracks, std::string &sdp)
    {
        json jsonSdp = json::object();
        json jsonMedias = json::array();

        std::string mids = "";

        // sdp: media
        for (auto &track : tracks) {
            jsonMedias.emplace_back(json::object());
            auto &jsonMedia = jsonMedias[jsonMedias.size() - 1];

            for (auto &transport : transports) {
                if (transport.mid != track.mid) {
                    continue;
                }

                // sdp: media: candidates
                json jsonCandidates = json::array();
                for (auto &candidate : transport.candidates) {
                    json jsonItem;
                    jsonItem["foundation"] = candidate.foundation;
                    jsonItem["ip"] = candidate.ip;
                    jsonItem["port"] = candidate.port;
                    jsonItem["priority"] = candidate.priority;
                    jsonItem["transport"] = candidate.protocol;
                    jsonItem["type"] = candidate.type;
                    jsonItem["component"] = candidate.component;
                    jsonCandidates.push_back(jsonItem);
                }

                // sdp: media: dtls
                auto &dtls = transport.dtls;
                jsonMedia["fingerprint"]["type"] = dtls.fingerprints[0].algorithm;
                jsonMedia["fingerprint"]["hash"] = dtls.fingerprints[0].value;
                jsonMedia["setup"] = dtls2SdpRole[dtls.role];

                // sdp: media: ice
                auto &ice = transport.ice;
                jsonMedia["icePwd"] = ice.pwd;
                jsonMedia["iceUfrag"] = ice.ufrag;
                jsonMedia["iceOptions"] = ice.options;

                break;
            }

            // sdp: media: ext
            int i = 0;
            jsonMedia["ext"] = json::array();
            auto jsonExtIt = jsonMedia.find("ext");
            for (auto &ext : track.rtpParameters.headerExtensions) {
                jsonExtIt->emplace_back(json::value_t::object);
                (*jsonExtIt)[i]["uri"] = ext.uri;
                (*jsonExtIt)[i]["value"] = ext.id;
                i++;
            }

            // sdp: media: fmtp & rtcpFb
            jsonMedia["fmtp"] = json::array();
            jsonMedia["rtcpFb"] = json::array();
            auto jsonFmtpIt = jsonMedia.find("fmtp");
            auto jsonRtcpFbIt = jsonMedia.find("rtcpFb");
            std::string payloads = "";
            i = 0;
            for (auto &codec : track.rtpParameters.codecs) {
                jsonFmtpIt->emplace_back(json::value_t::object);

                std::string parameters = "";
                json jsonParameters;
                codec.parameters.FillJson(jsonParameters);
                for (auto &jsonItem : jsonParameters.items()) {
                    auto &key = jsonItem.key();
                    auto &value = jsonItem.value();

                    if (value.is_number()) {
                        parameters += key + "=" + std::to_string(value.get<int>()) + ";";
                    } else if (value.is_string()) {
                        parameters += key + "=" + value.get<std::string>() + ";";
                    }
                }

                if (!parameters.empty() && parameters.at(parameters.size() - 1) == ';') {
                    parameters = parameters.substr(0, parameters.size() - 1);
                }

                (*jsonFmtpIt)[i]["config"] = parameters;
                (*jsonFmtpIt)[i]["payload"] = codec.payloadType;
                i++;

                if (codec.rtcpFeedback.size() > 0) {
                    int j = 0;
                    jsonRtcpFbIt->emplace_back(json::value_t::object);
                    for (auto &fb : codec.rtcpFeedback) {
                        (*jsonRtcpFbIt)[j]["payload"] = codec.payloadType;
                        (*jsonRtcpFbIt)[j]["type"] = fb.type;
                        (*jsonRtcpFbIt)[j]["subtype"] = fb.parameter;
                        j++;
                    }
                }

                payloads += std::to_string(codec.payloadType) + " ";
            }

            // sdp: media: payloads
            if (!payloads.empty() && payloads.at(payloads.size() - 1) == ' ') {
                payloads = payloads.substr(0, payloads.size() - 1);
            }
            jsonMedia["payloads"] = payloads;

            // sdp: media: rtp
            jsonMedia["rtp"] = json::array();
            auto jsonRtpIt = jsonMedia.find("rtp");
            i = 0;
            for (auto &rtp : track.rtpParameters.codecs) {
                jsonRtpIt->emplace_back(json::value_t::object);
                auto &jsonRtp = (*jsonRtpIt)[i];
                jsonRtp["payload"] = rtp.payloadType;
                jsonRtp["rate"] = rtp.clockRate;
                jsonRtp["codec"] = RTC::RtpCodecMimeType::subtype2String[rtp.mimeType.subtype];
                if (track.kind == RTC::Media::Kind::AUDIO && rtp.channels > 0) {
                    jsonRtp["encoding"] = rtp.channels;
                }
                i++;
            }

            // sdp: media: ssrcs
            if (track.direction.find("recv") != std::string::npos) {

                json jsonSsrcs = json::array();
                json jsonGroups = json::array();
                std::string ssrcGroup = "";
                for (auto encoding : track.rtpParameters.encodings) {
                    if (encoding.ssrc == 0) {
                        continue;
                    }

                    json jsonSsrc = json::object();
                    jsonSsrc["attribute"] = "cname";
                    jsonSsrc["id"] = encoding.ssrc;
                    jsonSsrc["value"] = track.rtpParameters.rtcp.cname;
                    jsonSsrcs.push_back(jsonSsrc);

                    jsonSsrc = json::object();
                    jsonSsrc["attribute"] = "msid";
                    jsonSsrc["id"] = encoding.ssrc;
                    jsonSsrc["value"] = std::string(DEFAULT_SDP_MSLABEL) + " " + std::string(DEFAULT_SDP_MSLABEL) + "-" + RTC::Media::GetString(track.kind);
                    jsonSsrcs.push_back(jsonSsrc);

                    jsonSsrc = json::object();
                    jsonSsrc["attribute"] = "mslabel";
                    jsonSsrc["id"] = encoding.ssrc;
                    jsonSsrc["value"] = std::string(DEFAULT_SDP_MSLABEL);
                    jsonSsrcs.push_back(jsonSsrc);

                    jsonSsrc = json::object();
                    jsonSsrc["attribute"] = "label";
                    jsonSsrc["id"] = encoding.ssrc;
                    jsonSsrc["value"] = std::string(DEFAULT_SDP_MSLABEL) + "-" + RTC::Media::GetString(track.kind);
                    jsonSsrcs.push_back(jsonSsrc);

                    if (!encoding.hasRtx) {
                        continue;
                    }

                    jsonSsrc = json::object();
                    jsonSsrc["attribute"] = "cname";
                    jsonSsrc["id"] = encoding.rtx.ssrc;
                    jsonSsrc["value"] = track.rtpParameters.rtcp.cname;
                    jsonSsrcs.push_back(jsonSsrc);

                    jsonSsrc = json::object();
                    jsonSsrc["attribute"] = "msid";
                    jsonSsrc["id"] = encoding.rtx.ssrc;
                    jsonSsrc["value"] = std::string(DEFAULT_SDP_MSLABEL) + " " + std::string(DEFAULT_SDP_MSLABEL) + "-" + RTC::Media::GetString(track.kind);
                    jsonSsrcs.push_back(jsonSsrc);

                    jsonSsrc = json::object();
                    jsonSsrc["attribute"] = "mslabel";
                    jsonSsrc["id"] = encoding.rtx.ssrc;
                    jsonSsrc["value"] = std::string(DEFAULT_SDP_MSLABEL);
                    jsonSsrcs.push_back(jsonSsrc);

                    jsonSsrc = json::object();
                    jsonSsrc["attribute"] = "label";
                    jsonSsrc["id"] = encoding.rtx.ssrc;
                    jsonSsrc["value"] = std::string(DEFAULT_SDP_MSLABEL) + "-" + RTC::Media::GetString(track.kind);
                    jsonSsrcs.push_back(jsonSsrc);

                    json jsonGroup;
                    jsonGroup["semantics"] = "FID";
                    jsonGroup["ssrcs"] = std::to_string(encoding.ssrc) + " " + std::to_string(encoding.rtx.ssrc);
                    jsonGroups.push_back(jsonGroup);
                }

                jsonMedia["ssrcs"] = jsonSsrcs;
                if (jsonGroups.size() > 0) {
                    jsonMedia["ssrcGroups"] = jsonGroups;
                }
                // sdp: media: direction
                jsonMedia["direction"] = "sendrecv";

            } else {
                // sdp: media: direction
                jsonMedia["direction"] = "recvonly";
            }

            // sdp: media: mid
            jsonMedia["mid"] = track.rtpParameters.mid;
            // sdp: media: port (ignore)
            jsonMedia["port"] = 9;
            // sdp: media: protocol
            jsonMedia["protocol"] = "UDP/TLS/RTP/SAVPF";

            jsonMedia["rtcp"]["address"] = "0.0.0.0";
            jsonMedia["rtcp"]["ipVer"] = 4;
            jsonMedia["rtcp"]["netType"] = "IN";
            jsonMedia["rtcp"]["port"] = 9;

            // sdp: media: rtcpMux
            jsonMedia["rtcpMux"] = "rtcp-mux";
            // sdp: media: rtcpRsize
            jsonMedia["rtcpRsize"] = "rtcp-rsize";
            // sdp: media: x-google-flag
    //        jsonMedia["xGoogleFlag"] = "conference";
            // sdp: media: type (audio or video)
            jsonMedia["type"] = track.kind;
            jsonMedia["connection"]["ip"] = "0.0.0.0";
            jsonMedia["connection"]["version"] = 4;
            jsonMedia["icelite"] = "ice-lite";

            if (!track.trackId.empty()) {
                jsonMedia["control"] = std::string("trackId=") + track.trackId;
            }

            mids += track.rtpParameters.mid + " ";
        }

        jsonSdp["media"] = jsonMedias;

        if (!mids.empty() && mids.at(mids.size() - 1) == ' ') {
            mids = mids.substr(0, mids.size() - 1);
        }

        json jsonGroup = json::object();
        jsonGroup["mids"] = mids;
        jsonGroup["type"] = "BUNDLE";

        jsonSdp["groups"].push_back(jsonGroup);

        jsonSdp["msidSemantic"]["semantic"] = "WMS";
        jsonSdp["msidSemantic"]["token"] = "*";
        jsonSdp["name"] = "-";
        jsonSdp["origin"]["address"] = "0.0.0.0";
        jsonSdp["origin"]["ipVer"] = 4;
        jsonSdp["origin"]["netType"] = "IN";
        jsonSdp["origin"]["sessionId"] = "-";
        jsonSdp["origin"]["sessionVersion"] = 2;
        jsonSdp["origin"]["username"] = DEFAULT_SDP_USERNAME;

        jsonSdp["timing"]["start"] = 0;
        jsonSdp["timing"]["stop"] = 0;

        jsonSdp["version"] = 0;

        sdp = sdptransform::write(jsonSdp);

        return 0;

    }

}
