#pragma once

#include <string>
#include <list>
#include <unordered_map>
#include "json.hpp"
#include "SdpInfo.hpp"

using json = nlohmann::json;

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
    class Listener {
    public:
        virtual void OnRtcSessionAck(RtcSession *rtcSession, json &jsonObject) = 0;
        virtual void OnRtcSessionEvent(RtcSession *rtcSession, json &jsonObject) = 0;
    };

public:
    enum Role {
        NONE,
        PUBLISHER,
        PLAYER
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

private:
    Role m_role;
    std::string m_sessionId { "" };
    std::string m_streamId { "" };
    std::string m_routerId { "" };
    std::string m_transportId { "" };
    std::string m_videoProducerId { "" };
    std::string m_audioProducerId { "" };
    std::string m_dataProducerId { "" };
    std::string m_videoConsumerId { "" };
    std::string m_audioConsumerId { "" };
    std::string m_dataConsumerId { "" };

    WebRtcTransportParameters m_rtcTransportParameters;
    std::vector<ProducerParameters> m_producerParameters;
    std::vector<ConsumerParameters> m_consumerParameters;

    json m_jsonIceParameters;
    json m_jsonDtlsParameters;

    std::list<Listener *> m_listeners;

    RtcWorker *m_worker;
    void *m_ctx;

private:
    std::unordered_map<uint64_t, Request> m_requestWaittingMap;
};

}
