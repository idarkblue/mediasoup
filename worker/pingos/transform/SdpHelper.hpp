#pragma once

#include "utils/JsonHelper.hpp"
#include "RTC/RtpDictionaries.hpp"

namespace pingos
{
    class SdpHelper {
    public:
        struct WebRtcConnect {
           struct DtlsParameters {
                std::string role { "" };
                struct Fingerprint {
                    std::string algorithm { "" };
                    std::string value { "" };
                };
                std::vector<Fingerprint> fingerprints;
            };

            struct IceParameters {
                std::string ufrag { "" };
                std::string pwd { "" };
                std::string options { "" };
                std::string iceRole { "" };
                bool iceLite { false };
            };

            struct Candidate {
                std::string foundation { "" };
                std::string ip { "" };
                uint64_t priority { 0 };
                std::string protocol { "" };
                uint16_t port { 0 };
                std::string type { "" };
                std::string tcpType { "" };
                uint32_t component { 0 };
            };

            DtlsParameters dtls;
            IceParameters ice;
            std::vector<Candidate> candidates;
            std::string mid;
        };

        struct Track {
            RTC::Media::Kind kind;
            std::string mid { "" };
            std::string direction { "" };
            RTC::RtpParameters rtpParameters;
            std::string trackId = { "" };
        };

    public:
        SdpHelper() = default;
        virtual ~SdpHelper() = default;

    public:
        static int ParseTracks(std::string &sdp, std::vector<Track> &tracks);
        static int ParseHeaderExtensions(json &jsonHeaderExtensions, std::vector<RTC::RtpHeaderExtensionParameters> &headerExtensions);
        static int ParseCodecs(json &jsonRtp, std::vector<RTC::RtpCodecParameters> &codecs);
        static int ParseEncodings(json &jsonRtp, std::vector<RTC::RtpEncodingParameters> &encodings);
        static int ParseRtcp(json &jsonRtp, RTC::RtcpParameters &rtcp);
        static int ParseWebRtcConnect(std::string &sdp, std::vector<WebRtcConnect> &parameters);

    public:
        static int ParseWebRtcTransport(json &jsonObject, WebRtcConnect &conn);

    public:
        static void AutoFillEncodings(Track &track);
        static void AutoFillRtcp(Track &track);
        static void ReverseTrack(Track &track);

    public:
        static int Transform2WebRtcTransportJson(WebRtcConnect &parameter, json &jsonObject);
        static int Transform2ProducerJson(Track &parameter, json &jsonObject);
        static int Transform2SimpleConsumerJson(Track &parameter, json &jsonObject);

    public:
        static int Transform2Sdp(std::vector<WebRtcConnect> &transports, std::vector<Track> &tracks, std::string &sdp);
    };
}
