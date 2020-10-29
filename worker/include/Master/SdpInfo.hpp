#ifndef _SDP_INFO_H_INCLUDE_
#define _SDP_INFO_H_INCLUDE_
#include <vector>
#include "json.hpp"
#include "RTC/RtpDictionaries.hpp"
#include "RTC/DtlsTransport.hpp"
#include "RTC/IceCandidate.hpp"

using json = nlohmann::json;

namespace pingos {

struct IceParameters {
    std::string usernameFragment { "" };
    std::string password { "" };
    bool iceLite { true };
    std::string protocol { "" };
    std::string ip { "" };
    int port { 0 };
    std::string tcpType { "" };
};

struct WebRtcTransportParameters {
    struct Fingerprint {
        std::string algorithm { "" };
        std::string value { "" };
    };
    struct  {
        RTC::DtlsTransport::Role role { RTC::DtlsTransport::Role::NONE };
        std::vector<Fingerprint> fingerprints;
    } dtlsParameters;

    struct LocalAddr {
        std::string ip { "" };
        std::string announcedIp { "" };
    };

    std::vector<LocalAddr> localAddrs;

    bool enableUdp { false };
    bool enableTcp { false };
    bool preferUdp { false };
    bool preferTcp { false };
    bool enableSctp { false };

    int initialAvailableOutgoingBitrate { 0 };
    struct {
        int os { 0 };
        int mis { 0 };
    } numSctpStreams;

    int maxSctpMessageSize { 0 };
    bool isDataChannel { 0 };
    std::string sessionId;
    std::string uesrName;

public:
    void AddLocalAddr(std::string ip, std::string announcedIp);
    void AddFingerprint(Fingerprint &fingerprint);
    void FillJsonTransport(json &jsonObject);
    void FillJsonDtls(json &jsonObject);
};

struct ProducerParameters {
    struct RtpEncodingMapping {
        std::string rid { "" };
        uint32_t ssrc { 0 };
        uint32_t mappedSsrc { 0 };
    };

    struct RtpMapping {
        std::map<uint8_t, uint8_t> codecs;
        std::vector<RtpEncodingMapping> encodings;
    };

    RtpMapping rtpMapping;
    RTC::RtpParameters rtpParameters;
    std::string kind { "" };
    bool paused { false };

    void FillJson(json &jsonObject);
};

struct ConsumerParameters {
    RTC::RtpParameters rtpParameters;
    std::vector<RTC::RtpEncodingParameters> consumableRtpEncodings;
    std::string kind { "" };
    std::string type { "simple" };
    bool paused { false };

    int SetRtpParameters(ProducerParameters &producer);

    void FillJson(json &jsonObject);
};

class SdpInfo {
public:
    SdpInfo(std::string sdp);
    virtual ~SdpInfo();

    // Producer
    int TransformSdp(WebRtcTransportParameters &rtcTransportParameters,
        std::vector<ProducerParameters> &producerParameters);

    // Consumer
    int TransformSdp(WebRtcTransportParameters &rtcTransportParameters,
        std::vector<ConsumerParameters> &consumerParameters);

protected:
    // webrtc transport parameters
    int ParseWebRtcTransport(json &jsonSdp, WebRtcTransportParameters &rtcTransportParameters);
    int ParseFingerprint(json &jsonFingerprint, WebRtcTransportParameters::Fingerprint &fingerprint);

    // producer parameters
    int ParseProducers(json &jsonSdp, std::vector<ProducerParameters> &producerParameters);
    int ParseRtpParameters(json &jsonRtp, RTC::RtpParameters &rtpParameters);
    int ParseRtpMapping(json &jsonRtpMapping, ProducerParameters::RtpMapping &rtpMapping);
    int ParseHeaderExtensions(json &jsonHeaderExtensions, std::vector<RTC::RtpHeaderExtensionParameters> &headerExtensions);
    int ParseCodecs(json &jsonRtp, std::vector<RTC::RtpCodecParameters> &codecs);
    int ParseEncodings(json &jsonRtp, std::vector<RTC::RtpEncodingParameters> &encodings);
    int ParseRtcp(json &jsonRtp, RTC::RtcpParameters &rtcp);
    int ParseRtpMapping(ProducerParameters &producer);

private:
    std::string m_sdp { "" };
    std::vector<ProducerParameters*> m_producerParameters;
};

}

#endif
