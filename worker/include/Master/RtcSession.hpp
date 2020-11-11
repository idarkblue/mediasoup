#pragma once

#include <string>
#include <list>
#include <unordered_map>
#include "json.hpp"
#include "SdpInfo.hpp"

namespace pingos {

class RtcSession;
class RtcWorker;

class RtcEvent {
public:
    enum EventId {
        PUBLISH,
        PLAY,
        MUTE,
        ICE,
        DTLS
    };

    static std::unordered_map<EventId, std::string> eventId2String;
    static std::unordered_map<std::string, EventId> string2EventId;

public:
    RtcEvent(std::string sessionId, std::string streamId, EventId eventId);
    virtual ~RtcEvent();

public:
    std::string sessionId;
    std::string streamId;
    std::string event;
    EventId eventId;
    json jsonData;
};

class RtcSession {
public:
    class Request {
    public:
        Request() = default;
        Request(const std::string method);
        virtual ~Request();

        void Init(const std::string method);
        uint64_t GetId();
        std::string GetMethod();

    public:
        void FillJson(json &jsonObject);

    public:
        void SetInternal(json &jsonObject);
        void SetData(json &jsonObject);

    private:
        uint64_t m_id { 0 };
        std::string m_method { "" };
        json m_jsonInternal;
        json m_jsonData;
        static uint64_t m_requestId;
    };

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
    };

public:
    enum Role {
        NONE,
        PUBLISHER,
        PLAYER,
        MONITOR
    };
    static std::string Role2String(Role role);

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
    void ReceiveChannelEvent(json &jsonObject);

public:
    int Publish(std::string sdp);
    int Play(std::string sdp);

    int Pause(std::string kind);
    int Resume(std::string kind);

    int Close();

public:
    int SetRemoteSdp(std::string sdp);
    int GetLocalSdp(std::string &sdp);

    void AddLocalAddress(std::string ip, std::string announcedIp);
    int SetProducerParameters(RtcSession &rtcSession);
    void SetContext(void *ctx);
    void* GetContext();

protected:
    int GenerateRouterRequest(std::string method, Request &request);
    int GenerateWebRtcTransportRequest(std::string method, Request &request);
    int GenerateProducerRequest(std::string method, std::string kind, Request &request);
    int GenerateConsumerRequest(std::string method, std::string kind, Request &request);

private:
    int ActiveRtcSessionRequest(Request &request);
    void ActiveRtcSessionEvent(Request &request, json &jsonObject);

    int FillOffer(std::string &sdp);
    int FillAnswer(std::string &sdp);
    int FillCandidates(json &jsonObject);

private:

    Role role;
    std::string sessionId { "" };
    std::string streamId { "" };
    std::string routerId { "" };
    std::string transportId { "" };
    std::string videoProducerId { "" };
    std::string audioProducerId { "" };
    std::string dataProducerId { "" };
    std::string videoConsumerId { "" };
    std::string audioConsumerId { "" };
    std::string dataConsumerId { "" };

    WebRtcTransportParameters rtcTransportParameters;
    std::vector<ProducerParameters> producerParameters;
    std::vector<ConsumerParameters> consumerParameters;

    json jsonIceParameters;
    json jsonIceCandidateParameters;
    json jsonDtlsParameters;

    std::list<Listener *> listeners;

    RtcWorker *worker;
    void *ctx;

    Status status { Status::IDLE };

private:
    std::unordered_map<uint64_t, Request> requestWaittingMap;
};

}
