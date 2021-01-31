#pragma once

#include <string>
#include <list>
#include <unordered_map>
#include "json.hpp"
#include "SdpInfo.hpp"
#include "Worker/ChannelRequest.hpp"
#include "Worker/PlainTransportConstructor.hpp"

namespace pingos {

class RtcSession;
class RtcWorker;

class RtcSession {

public:
    enum Status {
        IDLE,
        INITED,
        TRANSPORT_CREATED,
        MEDIA_CREATED,
        CLOSED
    };

public:
    class Listener {
    public:
        virtual void OnRtcSessionAck(RtcSession *rtcSession, json &jsonObject) = 0;
        virtual void OnRtcSessionEvent(RtcSession *rtcSession, json &jsonObject) = 0;
        virtual void OnRtcSessionClosed(RtcSession *rtcSession) = 0;
    };

public:
    enum Role {
        NONE,
        PUBLISHER,
        PLAYER,
        RTSP_PLAYER,
        MONITOR
    };
    static std::string Role2String(Role role);
    static RtcSession::Role String2Role(std::string role);

public:
    RtcSession(Role role, std::string sessionId, std::string stream);
    virtual ~RtcSession();

    void AddListener(Listener *listener);
    void SetWorker(RtcWorker *worker);

    Role GetRole();
    std::string GetSessionId();
    std::string GetStreamId();
    std::string GetTransportId();
    std::string GetProducerId(std::string kind);
    std::string GetConsumerId(std::string kind);
    std::vector<ProducerParameters> &GetProducerParameters();

public:
    void ReceiveChannelAck(json &jsonObject);

public:
    int Publish(std::string sdp);
    int Play(std::string sdp);
    int Pause(std::string kind);
    int Resume(std::string kind);
    int Close();

public:
    int SetProducerParameters(std::vector<ProducerParameters> &producerParameters);
    int SetConsumerParameters(std::vector<ConsumerParameters> &consumerParameters);
    int CreatePlainTransport(PlainTransportConstructor &plainTransportParameters);
    int ConnectPlainTransport(std::string ip, uint16_t port, uint16_t rtcpPort, uint16_t trackId);
    int TrackPlay(std::string kind, uint16_t trackId);
    int TrackPublish(std::string kind, uint16_t trackId);

public:
    void AddLocalAddress(std::string ip, std::string announcedIp);
    int SetProducerParameters(RtcSession &rtcSession);
    void SetContext(void *ctx);
    void* GetContext();

protected:
    int CloseTransport(std::string transportId);

protected:
    int GenerateRouterRequest(std::string method, ChannelRequest &request);
    int GenerateWebRtcTransportRequest(std::string method, ChannelRequest &request);
    int GenerateProducerRequest(std::string method, std::string kind, ChannelRequest &request, std::string trackId = "");
    int GenerateConsumerRequest(std::string method, std::string kind, ChannelRequest &request, std::string trackId = "");

private:
    int ActiveRtcSessionRequest(ChannelRequest &request);
    void ActiveRtcSessionEvent(ChannelRequest &request, json &jsonObject);

    int FillOffer(std::string &sdp);
    int FillAnswer(std::string &sdp);
    int FillCandidates(json &jsonObject);

private:
    void FireEvent(std::string event, json& data);

private:
    Role role;
    std::string sessionId { "" };
    std::string streamId { "" };
    std::string routerId { "" };
    std::string videoProducerId { "" };
    std::string audioProducerId { "" };
    std::string dataProducerId { "" };
    std::string videoConsumerId { "" };
    std::string audioConsumerId { "" };
    std::string dataConsumerId { "" };

    WebRtcTransportParameters rtcTransportParameters;
    std::vector<ProducerParameters> producerParameters;
    std::vector<ConsumerParameters> consumerParameters;
    std::vector<std::string> transportIds;

    json jsonIceParameters;
    json jsonIceCandidateParameters;
    json jsonDtlsParameters;

    std::list<Listener *> listeners;

    RtcWorker *worker;
    void *ctx;

    Status status { Status::IDLE };

    std::string transportType;

private:
    std::unordered_map<uint64_t, ChannelRequest> requestWaittingMap;
};

}
