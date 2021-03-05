#pragma once

#include <string>
#include <vector>
#include "RTC/RtpDictionaries.hpp"
#include "json.hpp"

using json = nlohmann::json;

namespace pingos
{
    class RtpTrack {
    public:
        RtpTrack();
        virtual ~RtpTrack();

    public:
        struct EncodingParameters {
            bool active { false };
            uint32_t maxBitrate { 0 };
            uint32_t scaleResolutionDownBy { 0 };
            std::string rid { "" };
            std::string ssrc { "" };
            std::string scalabilityMode { "" };
        };
        struct DtlsParameters {
            std::string setup;
            std::map<std::string, std::string> fingerprintmap;
        };
        struct IceParameters {
            std::string ufrag { "" };
            std::string pwd { "" };
            std::string options { "" };
        };
        struct IceCandidate {
            std::string fondation { "" };
            std::string protocol { "" };
            std::string ip { "" };
            std::string type { "" };
            uint16_t port { 0 };
        };
        RTC::Media::Kind kind;
        std::string mid { "" };
        std::string trackId { "" };
        uint32_t payloadType { 0 };
        uint32_t clockRate { 0 };
        std::string mimeType { "" };
        std::map<std::string, std::string> rtpmap;
        std::map<std::string, uint32_t> fmtp;
        std::map<uint32_t, std::string> extmap;
        std::map<std::string, std::string> fbmap;
        std::shared_ptr<IceParameters> ice { nullptr };
        std::shared_ptr<DtlsParameters> dtls { nullptr };

        std::vector<Encoding> encodings;
    };
}
