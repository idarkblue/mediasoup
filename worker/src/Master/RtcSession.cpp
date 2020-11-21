#include "Master/Log.hpp"
#include "Master/RtcSession.hpp"
#include "Master/RtcWorker.hpp"
#include "Channel/Request.hpp"
#include "Master/sdptransform.hpp"

#define PMS_CLASS "pingos::RtcSession"

namespace pingos {

static std::string OfferMslabel = "pingos-rtc-ms-label";
/*
std::unordered_map<RtcEvent::EventId, std::string> RtcEvent::eventId2String = {
    { RtcEvent::EventId::PUBLISH, "publish" },
    { RtcEvent::EventId::PLAY, "play" },
    { RtcEvent::EventId::MUTE, "mute" },
    { RtcEvent::EventId::ICE, "ice" },
    { RtcEvent::EventId::DTLS, "dtls" }
};
std::unordered_map<std::string, RtcEvent::EventId> RtcEvent::string2EventId = {
    { "publish", RtcEvent::EventId::PUBLISH },
    { "play", RtcEvent::EventId::PLAY },
    { "mute", RtcEvent::EventId::MUTE },
    { "ice", RtcEvent::EventId::ICE },
    { "dtls", RtcEvent::EventId::DTLS }
};

RtcEvent::RtcEvent(std::string sessionId, std::string streamId, EventId eventId)
{
    eventId2String[eventId];
}

RtcEvent::~RtcEvent()
{

}
*/
std::string RtcSession::Role2String(RtcSession::Role role)
{
    static std::string role2string[] = {
        "none",
        "publisher",
        "player",
        "rtsp_player",
        "monitor"
    };

    return role2string[role];
}

RtcSession::Role RtcSession::String2Role(std::string role)
{
    static std::map<std::string, RtcSession::Role> string2role = {
        { "none", RtcSession::Role::NONE },
        { "publisher", RtcSession::Role::PUBLISHER },
        { "player", RtcSession::Role::PLAYER },
        { "rtsp_player", RtcSession::Role::RTSP_PLAYER },
        { "monitor", RtcSession::Role::MONITOR }
    };

    if (string2role.find(role) == string2role.end()) {
        return RtcSession::Role::NONE;
    }

    return string2role[role];
}

RtcSession::RtcSession(Role role, std::string sessionId, std::string stream):
    role(role), sessionId(sessionId), streamId(stream)
{
    static uint64_t guid = 0;
    this->routerId = "pingos";
    this->transportId = sessionId + std::string("-")
        + Role2String(role) + std::string("-")
        + stream + std::string("-") + std::to_string(guid);

    if (role == Role::PUBLISHER) {
        this->videoProducerId = this->transportId + std::string("-") + std::string("video");
        this->audioProducerId = this->transportId + std::string("-") + std::string("audio");
    } else {
        this->videoConsumerId = this->transportId + std::string("-") + std::string("video");
        this->audioConsumerId = this->transportId + std::string("-") + std::string("audio");
    }
}

RtcSession::~RtcSession()
{
    this->Close();

    json jsonData = json::object();
    jsonData["sessionState"] = "closed";
    this->FireEvent("sessionstatechange", jsonData);
}

void RtcSession::AddListener(RtcSession::Listener *listener)
{
    this->listeners.push_back(listener);
}

void RtcSession::SetWorker(RtcWorker *worker)
{
    this->worker = worker;
    this->status = Status::INITED;
}

RtcSession::Role RtcSession::GetRole()
{
    return this->role;
}

std::string RtcSession::GetSessionId()
{
    return this->sessionId;
}

std::string RtcSession::GetStreamId()
{
    return this->streamId;
}

std::string RtcSession::GetTransportId()
{
    return this->transportId;
}

std::string RtcSession::GetProducerId(std::string kind)
{
    if (kind == "video") {
        return this->videoProducerId;
    } else if (kind == "audio") {
        return this->audioProducerId;
    }

    return "";
}

std::string RtcSession::GetConsumerId(std::string kind)
{
    if (kind == "video") {
        return this->videoConsumerId;
    } else if (kind == "audio") {
        return this->audioConsumerId;
    }

    return "";
}

std::vector<ProducerParameters> &RtcSession::GetProducerParameters()
{
    return this->producerParameters;
}

void RtcSession::ReceiveChannelAck(json &jsonObject)
{
    auto id = jsonObject["id"].get<uint64_t>();
    auto it = this->requestWaittingMap.find(id);

    if (it == this->requestWaittingMap.end()) {
        PMS_ERROR("SessionId[{}] streamId[{}] Invalid ack, unknown request id[{}]",
            this->sessionId, this->streamId, id);
        return;
    }

    auto &request = it->second;
    auto jsonErrorIt = jsonObject.find("error");
    auto jsonAcceptedIt = jsonObject.find("accepted");
    std::string sdp;
    std::string ack;
    switch (Channel::Request::string2MethodId[request.GetMethod()]) {
        case Channel::Request::MethodId::ROUTER_CREATE_PLAIN_TRANSPORT:
            ack = "plainTransport";
        break;

        case Channel::Request::MethodId::ROUTER_CREATE_WEBRTC_TRANSPORT:
        if (jsonAcceptedIt != jsonObject.end()) {
            this->jsonIceParameters = jsonObject["data"]["iceParameters"];
            this->jsonIceCandidateParameters = jsonObject["data"]["iceCandidates"];
            this->jsonDtlsParameters = jsonObject["data"]["dtlsParameters"];
        }
//        case Channel::Request::MethodId::TRANSPORT_CONNECT:
//        case Channel::Request::MethodId::TRANSPORT_PRODUCE:
//        case Channel::Request::MethodId::TRANSPORT_CONSUME:
        if (this->role == Role::PUBLISHER) {
            ack = "publish";
            if (this->FillAnswer(sdp) != 0) {
                jsonObject["error"] = 1;
                jsonObject["reason"] = "create answer sdp failed";
                jsonErrorIt = jsonObject.find("error");
            }
        } else if (this->role == Role::PLAYER) {
            ack = "play";
            if (this->FillOffer(sdp) != 0) {
                jsonObject["error"] = 1;
                jsonObject["reason"] = "create answer sdp failed";
                jsonErrorIt = jsonObject.find("error");
            }
        }
        break;

        default:
        PMS_INFO("SessionId[{}] StreamId[{}] ignore ack[{}] response",
            this->sessionId, this->streamId, request.GetMethod());
        return;
        break;
    }

    json jsonAck = json::object();
    jsonAck["sessionId"] = this->sessionId;
    jsonAck["streamId"] = this->streamId;
    jsonAck["ack"] = ack;
    if (jsonErrorIt != jsonObject.end()) {
        std::string reason;
        jsonAck["error"] = 1;
        JSON_READ_VALUE_DEFAULT(jsonObject, "reason", std::string, reason, "");
        jsonAck["reason"] = reason;
    } else {
        if (!sdp.empty()) {
            jsonObject = json::object();
            jsonObject["data"]["sdp"] = sdp;
        }

        jsonAck["error"] = 0;
        jsonAck["reason"] = "OK";
        jsonAck["data"] = jsonObject["data"];
        this->status = Status::TRANSPORT_CREATED;
    }

    for (auto listener : this->listeners) {
        listener->OnRtcSessionAck(this, jsonAck);
    }
}

int RtcSession::Publish(std::string sdp)
{
    SdpInfo si(sdp);

    if (si.TransformSdp(this->rtcTransportParameters, this->producerParameters) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] publish failed, Transform sdp error",
            this->sessionId, this->streamId);
        return -1;
    }

    ChannelRequest request;
    if (GenerateRouterRequest("worker.createRouter", request) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] publish failed, generate router request error",
            this->sessionId, this->streamId);
        return -1;
    }

    if (ActiveRtcSessionRequest(request) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] publish failed, run router request error",
            this->sessionId, this->streamId);
        return -1;
    }

    if (GenerateWebRtcTransportRequest("router.createWebRtcTransport", request) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] publish failed, generate webrtctransport request error",
            this->sessionId, this->streamId);
        return -1;
    }

    if (ActiveRtcSessionRequest(request) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] publish failed, run webrtctransport request error",
            this->sessionId, this->streamId);
        return -1;
    }

    if (GenerateWebRtcTransportRequest("transport.connect", request) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] publish failed, transport connect request error",
            this->sessionId, this->streamId);
        return -1;
    }

    if (ActiveRtcSessionRequest(request) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] publish failed, run transport connect request error",
            this->sessionId, this->streamId);
        return -1;
    }

    for (auto &producer : this->producerParameters) {
        if (GenerateProducerRequest("transport.produce", producer.kind, request) != 0) {
            PMS_ERROR("SessionId[{}] StreamId[{}] publish failed, generate producer[{}] request error",
                this->sessionId, this->streamId, producer.kind);
            return -1;
        }
        if (ActiveRtcSessionRequest(request) != 0) {
            PMS_ERROR("SessionId[{}] StreamId[{}] publish failed, run producer[{}] request error",
                this->sessionId, this->streamId, producer.kind);
            return -1;
        }
    }

    PMS_INFO("SessionId[{}] StreamId[{}] publish success", this->sessionId, this->streamId);

    return 0;
}

int RtcSession::Play(std::string sdp)
{
    SdpInfo si(sdp);

    if (si.TransformSdp(this->rtcTransportParameters, this->consumerParameters) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] play failed, Transform sdp error",
            this->sessionId, this->streamId);
        return -1;
    }

    for (auto &producer : this->producerParameters) {
        for (auto &consumer : this->consumerParameters) {
            if (producer.kind != consumer.kind) {
                continue;
            }

            if (consumer.SetRtpParameters(producer) != 0) {
                PMS_ERROR("SessionId[{}] StreamId[{}] Set consumer's rtp parameters failed",
                    this->sessionId, this->streamId);
                return -1;
            }
        }
    }

    // for (auto it = this->consumerParameters.begin(); it != this->consumerParameters.end();) {
    //     if (it->rtpParameters.encodings.size() == 0) {
    //         this->consumerParameters.erase(it);
    //     } else {
    //         it++;
    //     }
    // }

    ChannelRequest request;
    if (GenerateRouterRequest("worker.createRouter", request) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] play failed, generate router request error",
            this->sessionId, this->streamId);
        return -1;
    }

    if (ActiveRtcSessionRequest(request) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] play failed, run router request error",
            this->sessionId, this->streamId);
        return -1;
    }

    if (GenerateWebRtcTransportRequest("router.createWebRtcTransport", request) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] play failed, generate webrtctransport request error",
            this->sessionId, this->streamId);
        return -1;
    }

    if (ActiveRtcSessionRequest(request) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] play failed, run webrtctransport request error",
            this->sessionId, this->streamId);
        return -1;
    }

    if (GenerateWebRtcTransportRequest("transport.connect", request) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] play failed, transport connect request error",
            this->sessionId, this->streamId);
        return -1;
    }

    if (ActiveRtcSessionRequest(request) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] play failed, run transport connect request error",
            this->sessionId, this->streamId);
        return -1;
    }

    for (auto &consumer : this->consumerParameters) {
        if (GenerateConsumerRequest("transport.consume", consumer.kind, request) != 0) {
            PMS_ERROR("SessionId[{}] StreamId[{}] play failed, generate consumer[{}] request error",
                this->sessionId, this->streamId, consumer.kind);
            return -1;
        }
        if (ActiveRtcSessionRequest(request) != 0) {
            PMS_ERROR("SessionId[{}] StreamId[{}] play failed, consumer[{}] run request error",
                this->sessionId, this->streamId, consumer.kind);
            return -1;
        }
    }

    PMS_INFO("SessionId[{}] StreamId[{}] play success", this->sessionId, this->streamId);

    return 0;
}

int RtcSession::Pause(std::string kind)
{
    ChannelRequest request;

    for (auto &consumer : this->consumerParameters) {
        if (kind != consumer.kind) {
            continue;
        }

        if (GenerateConsumerRequest("consume.pause", consumer.kind, request) != 0) {
            PMS_ERROR("SessionId[{}] StreamId[{}] pause failed, generate consumer[{}] request error",
                this->sessionId, this->streamId, consumer.kind);
            return -1;
        }

        if (ActiveRtcSessionRequest(request) != 0) {
            PMS_ERROR("SessionId[{}] StreamId[{}] pause failed, consumer[{}] run request error",
                this->sessionId, this->streamId, consumer.kind);
            return -1;
        }
    }

    return 0;
}

int RtcSession::Resume(std::string kind)
{
    ChannelRequest request;

    for (auto &consumer : this->consumerParameters) {
        if (kind != consumer.kind) {
            continue;
        }

        if (GenerateConsumerRequest("consume.resume", consumer.kind, request) != 0) {
            PMS_ERROR("SessionId[{}] StreamId[{}] resume failed, generate consumer[{}] request error",
                this->sessionId, this->streamId, consumer.kind);
            return -1;
        }

        if (ActiveRtcSessionRequest(request) != 0) {
            PMS_ERROR("SessionId[{}] StreamId[{}] resume failed, consumer[{}] run request error",
                this->sessionId, this->streamId, consumer.kind);
            return -1;
        }
    }

    return 0;
}

int RtcSession::Close()
{
    if (this->status == Status::CLOSED) {
        PMS_WARN("SessionId[{}] StreamId[{}] the session has been closed",
            this->sessionId, this->streamId);
        return 0;
    }

    if (this->role != Role::PUBLISHER && this->role != Role::PLAYER) {
        return 0;
    }

    ChannelRequest request;

    if (GenerateWebRtcTransportRequest("transport.close", request) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] close failed, GenerateWebRtcTransportRequest error",
            this->sessionId, this->streamId);
        return -1;
    }

    if (ActiveRtcSessionRequest(request) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] close failed, run request error",
            this->sessionId, this->streamId);
        return -1;
    }

    this->status = Status::CLOSED;

    return 0;
}

int RtcSession::SetConsumerParameters(std::vector<ConsumerParameters> &consumerParameters)
{
    this->consumerParameters = consumerParameters;

    return 0;
}

int RtcSession::CreatePlainTransport(PlainTransportConstructor &plainTransportParameters)
{
    ChannelRequest request;
    request.Init("router.createPlainTransport");

    json jsonInternal;
    jsonInternal["routerId"] = this->routerId;
    jsonInternal["transportId"] = this->transportId + std::string("-") + std::to_string(plainTransportParameters.trackId);

    request.SetInternal(jsonInternal);

    json jsonData;
    plainTransportParameters.FillJson(jsonData);
    request.SetData(jsonData);

    if (ActiveRtcSessionRequest(request) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] play failed, run transport connect request error",
            this->sessionId, this->streamId);
        return -1;
    }

    return 0;
}

int RtcSession::ConnectPlainTransport(std::string ip, uint16_t port, uint16_t rtcpPort, uint16_t trackId)
{
    ChannelRequest request;
    request.Init("transport.connect");

    json jsonInternal;
    jsonInternal["routerId"] = this->routerId;
    jsonInternal["transportId"] = this->transportId + std::string("-") + std::to_string(trackId);

    json jsonData;
    jsonData["ip"] = ip;
    jsonData["port"] = port;
    jsonData["rtcpPort"] = rtcpPort;

    request.SetData(jsonData);

    if (ActiveRtcSessionRequest(request) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] play failed, run transport connect request error",
            this->sessionId, this->streamId);
        return -1;
    }

    return 0;
}

int RtcSession::TrackPlay(std::string kind, uint16_t trackId)
{
    ChannelRequest request;

    for (auto &consumer : this->consumerParameters) {
        if (consumer.kind != kind)
        {
            continue;
        }

        if (GenerateConsumerRequest("transport.consume", consumer.kind, request, std::to_string(trackId)) != 0) {
            PMS_ERROR("SessionId[{}] StreamId[{}] play failed, generate consumer[{}] request error",
                this->sessionId, this->streamId, consumer.kind);
            return -1;
        }

        if (ActiveRtcSessionRequest(request) != 0) {
            PMS_ERROR("SessionId[{}] StreamId[{}] play failed, consumer[{}] run request error",
                this->sessionId, this->streamId, consumer.kind);
            return -1;
        }
    }

    PMS_INFO("SessionId[{}] StreamId[{}] play success", this->sessionId, this->streamId);

    return 0;
}

void RtcSession::AddLocalAddress(std::string ip, std::string announcedIp)
{
    this->rtcTransportParameters.AddLocalAddr(ip, announcedIp);
}

int RtcSession::SetProducerParameters(RtcSession &rtcSession)
{
    this->producerParameters = rtcSession.GetProducerParameters();

    this->videoProducerId = rtcSession.GetProducerId("video");
    this->audioProducerId = rtcSession.GetProducerId("audio");

    return 0;
}

void RtcSession::SetContext(void *ctx)
{
    this->ctx = ctx;
}

void* RtcSession::GetContext()
{
    return this->ctx;
}

int RtcSession::GenerateRouterRequest(std::string method, ChannelRequest &request)
{
    request.Init(method);

    if (Channel::Request::string2MethodId.count(method) == 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] Invalid method, unknown method {}",
            this->sessionId, this->streamId, method);
        return -1;
    }

    json jsonInternal = json::object();
    json jsonData = json::object();

    jsonInternal["routerId"] = this->routerId;

    switch (Channel::Request::string2MethodId[method]) {
        case Channel::Request::MethodId::WORKER_CREATE_ROUTER:
        break;
        case Channel::Request::MethodId::ROUTER_DUMP:
        break;
        case Channel::Request::MethodId::ROUTER_CLOSE:
        break;
        default:
        PMS_ERROR("SessionId[{}] StreamId[{}] Invalid method, method {} not supportted",
            this->sessionId, this->streamId, method);
        return -1;
    }

    PMS_DEBUG("internal {}", jsonInternal.dump());

    request.SetInternal(jsonInternal);
    request.SetData(jsonData);

    return 0;
}

int RtcSession::GenerateWebRtcTransportRequest(std::string method, ChannelRequest &request)
{
    request.Init(method);

    if (Channel::Request::string2MethodId.count(method) == 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] Invalid method, unknown method {}",
            this->sessionId, this->streamId, method);
        return -1;
    }
    json jsonInternal = json::object();
    json jsonData = json::object();

    jsonInternal["routerId"] = this->routerId;
    jsonInternal["transportId"] = this->transportId;

    switch (Channel::Request::string2MethodId[method]) {
        case Channel::Request::MethodId::ROUTER_CREATE_WEBRTC_TRANSPORT:
        this->rtcTransportParameters.FillJsonTransport(jsonData);
        break;

        case Channel::Request::MethodId::TRANSPORT_CONNECT:
        this->rtcTransportParameters.FillJsonDtls(jsonData);
        break;

        case Channel::Request::MethodId::TRANSPORT_GET_STATS:
        case Channel::Request::MethodId::TRANSPORT_DUMP:
        case Channel::Request::MethodId::TRANSPORT_CLOSE:
        break;

        default:
        PMS_ERROR("SessionId[{}] StreamId[{}] Invalid method, method {} not supportted",
            this->sessionId, this->streamId, method);
        return -1;
    }

    request.SetInternal(jsonInternal);
    request.SetData(jsonData);

    return 0;
}

int RtcSession::GenerateProducerRequest(std::string method, std::string kind, ChannelRequest &request)
{
    request.Init(method);

    if (Channel::Request::string2MethodId.count(method) == 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] Invalid method, unknown method {}",
            this->sessionId, this->streamId, method);
        return -1;
    }

    std::string producerId = "";
    if (kind == "video") {
        producerId = this->videoProducerId;
    } else if (kind == "audio") {
        producerId = this->audioProducerId;
    } else {
        PMS_ERROR("SessionId[{}] StreamId[{}] Invalid kind {}", this->sessionId, this->streamId, kind);
        return -1;
    }

    json jsonInternal = json::object();
    json jsonData = json::object();

    jsonInternal["routerId"] = this->routerId;
    jsonInternal["transportId"] = this->transportId;
    jsonInternal["producerId"] = producerId;

    switch (Channel::Request::string2MethodId[method]) {
        case Channel::Request::MethodId::TRANSPORT_PRODUCE:
        for (auto &producer : this->producerParameters) {
            if (producer.kind == kind) {
                producer.FillJson(jsonData);
                break;
            }
        }
        break;

        case Channel::Request::MethodId::PRODUCER_DUMP:
        case Channel::Request::MethodId::PRODUCER_CLOSE:
        case Channel::Request::MethodId::PRODUCER_GET_STATS:
        case Channel::Request::MethodId::PRODUCER_PAUSE:
        case Channel::Request::MethodId::PRODUCER_RESUME:
        break;

        default:
        PMS_ERROR("SessionId[{}] StreamId[{}] Invalid method, method {} not supportted",
            this->sessionId, this->streamId, method);
        return -1;
    }

    request.SetInternal(jsonInternal);
    request.SetData(jsonData);

    return 0;
}

int RtcSession::GenerateConsumerRequest(std::string method, std::string kind, ChannelRequest &request, std::string trackId)
{
    request.Init(method);

    if (Channel::Request::string2MethodId.count(method) == 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] Invalid method, unknown method {}",
            this->sessionId, this->streamId, method);
        return -1;
    }

    std::string consumerId = "";
    std::string producerId = "";

    if (kind == "video") {
        consumerId = this->videoConsumerId;
        producerId = this->videoProducerId;
    } else if (kind == "audio") {
        consumerId = this->audioConsumerId;
        producerId = this->audioProducerId;
    } else {
        PMS_ERROR("SessionId[{}] StreamId[{}] Invalid kind {}", this->sessionId, this->streamId, kind);
        return -1;
    }

    json jsonInternal = json::object();
    json jsonData = json::object();

    jsonInternal["routerId"] = this->routerId;
    if (trackId.empty()) {
        jsonInternal["transportId"] = this->transportId;
    } else {
        jsonInternal["transportId"] = this->transportId + std::string("-") + trackId;
    }
    jsonInternal["consumerId"] = consumerId;
    jsonInternal["producerId"] = producerId;

    switch (Channel::Request::string2MethodId[method]) {
        case Channel::Request::MethodId::TRANSPORT_CONSUME:
        for (auto &consumer : this->consumerParameters) {
            if (consumer.kind == kind) {
                consumer.FillJson(jsonData);
                break;
            }
        }
        break;

        case Channel::Request::MethodId::CONSUMER_DUMP:
        case Channel::Request::MethodId::CONSUMER_CLOSE:
        case Channel::Request::MethodId::CONSUMER_GET_STATS:
        case Channel::Request::MethodId::CONSUMER_PAUSE:
        case Channel::Request::MethodId::CONSUMER_RESUME:
        break;

        default:
        PMS_ERROR("SessionId[{}] StreamId[{}] Invalid method, method {} not supportted",
            this->sessionId, this->streamId, method);
        return -1;
    }

    request.SetInternal(jsonInternal);
    request.SetData(jsonData);

    return 0;
}

int RtcSession::ActiveRtcSessionRequest(ChannelRequest &request)
{
    if (this->worker->SendRequest(this, request) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] run request failed",
            this->sessionId, this->streamId);

        return -1;
    }

    this->requestWaittingMap[request.GetId()] = request;

    return 0;
}

int RtcSession::FillOffer(std::string &sdp)
{
    if (this->jsonIceParameters.is_null() ||
        this->jsonIceCandidateParameters.is_null() ||
        this->jsonDtlsParameters.is_null())
    {
        PMS_ERROR("SessionId[{}] StreamId[{}] Ice dtls parameters needed",
            this->sessionId, this->streamId);
        return -1;
    }

    json jsonSdp = json::object();
    json jsonMedias = json::array();

    std::string mids = "";

    // sdp: media
    for (auto &consumer : this->consumerParameters) {
        jsonMedias.emplace_back(json::object());
        auto &jsonMedia = jsonMedias[jsonMedias.size() - 1];

        int i = 0;

        // sdp: media: candidates
        if (this->FillCandidates(jsonMedia["candidates"]) != 0) {
            PMS_ERROR("SessionId[{}] StreamId[{}] Fill ice candidate failed",
                this->sessionId, this->streamId);
            return -1;
        }

        // sdp: media: ext
        jsonMedia["ext"] = json::array();
        auto jsonExtIt = jsonMedia.find("ext");
        for (auto &ext : consumer.rtpParameters.headerExtensions) {
            jsonExtIt->emplace_back(json::value_t::object);
            (*jsonExtIt)[i]["uri"] = ext.uri;
            (*jsonExtIt)[i]["value"] = ext.id;
            i++;
        }

        // sdp: media: fingerprint
        if (this->rtcTransportParameters.dtlsParameters.fingerprints.size() > 0) {
            for (auto &jsonItem : this->jsonDtlsParameters["fingerprints"]) {
                if (jsonItem["algorithm"].get<std::string>()
                    == this->rtcTransportParameters.dtlsParameters.fingerprints[0].algorithm)
                {
                    jsonMedia["fingerprint"]["type"] = jsonItem["algorithm"].get<std::string>();
                    jsonMedia["fingerprint"]["hash"] = jsonItem["value"].get<std::string>();
                    break;
                }
            }
        }

        // sdp: media: ice
        jsonMedia["icePwd"] = this->jsonIceParameters["password"].get<std::string>();
        jsonMedia["iceUfrag"] = this->jsonIceParameters["usernameFragment"].get<std::string>();

        // sdp: media: setup
        if (this->jsonDtlsParameters["role"] == "server") {
            jsonMedia["setup"] = "passive";
        } else if (this->jsonDtlsParameters["role"] == "client") {
            jsonMedia["setup"] = "active";
        } else if (this->jsonDtlsParameters["role"] == "auto") {
            jsonMedia["setup"] = "active";
        }

        // sdp: media: fmtp & rtcpFb
        jsonMedia["fmtp"] = json::array();
        jsonMedia["rtcpFb"] = json::array();
        auto jsonFmtpIt = jsonMedia.find("fmtp");
        auto jsonRtcpFbIt = jsonMedia.find("rtcpFb");
        std::string payloads = "";
        i = 0;
        for (auto &codec : consumer.rtpParameters.codecs) {
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
                    if (!fb.parameter.empty()) {
                        (*jsonRtcpFbIt)[j]["subtype"] = fb.parameter;
                    }
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
        for (auto &rtp : consumer.rtpParameters.codecs) {
            jsonRtpIt->emplace_back(json::value_t::object);
            auto &jsonRtp = (*jsonRtpIt)[i];
            jsonRtp["payload"] = rtp.payloadType;
            jsonRtp["rate"] = rtp.clockRate;
            jsonRtp["codec"] = RTC::RtpCodecMimeType::subtype2String[rtp.mimeType.subtype];
            if (consumer.kind == "audio" && rtp.channels > 0) {
                jsonRtp["encoding"] = rtp.channels;
            }
            i++;
        }

        // sdp: media: ssrcs
        json jsonSsrcs = json::array();
        json jsonGroups = json::array();
        std::string ssrcGroup = "";
        for (auto encoding : consumer.rtpParameters.encodings) {
            if (encoding.ssrc == 0) {
                continue;
            }

            json jsonSsrc = json::object();
            jsonSsrc["attribute"] = "cname";
            jsonSsrc["id"] = encoding.ssrc;
            jsonSsrc["value"] = consumer.rtpParameters.rtcp.cname;
            jsonSsrcs.push_back(jsonSsrc);

            jsonSsrc = json::object();
            jsonSsrc["attribute"] = "msid";
            jsonSsrc["id"] = encoding.ssrc;
            jsonSsrc["value"] = OfferMslabel + " " + OfferMslabel + "-" + consumer.kind;
            jsonSsrcs.push_back(jsonSsrc);

            jsonSsrc = json::object();
            jsonSsrc["attribute"] = "mslabel";
            jsonSsrc["id"] = encoding.ssrc;
            jsonSsrc["value"] = OfferMslabel;
            jsonSsrcs.push_back(jsonSsrc);

            jsonSsrc = json::object();
            jsonSsrc["attribute"] = "label";
            jsonSsrc["id"] = encoding.ssrc;
            jsonSsrc["value"] = OfferMslabel + "-" + consumer.kind;
            jsonSsrcs.push_back(jsonSsrc);

            if (!encoding.hasRtx) {
                continue;
            }

            jsonSsrc = json::object();
            jsonSsrc["attribute"] = "cname";
            jsonSsrc["id"] = encoding.rtx.ssrc;
            jsonSsrc["value"] = consumer.rtpParameters.rtcp.cname;
            jsonSsrcs.push_back(jsonSsrc);

            jsonSsrc = json::object();
            jsonSsrc["attribute"] = "msid";
            jsonSsrc["id"] = encoding.rtx.ssrc;
            jsonSsrc["value"] = OfferMslabel + " " + OfferMslabel + "-" + consumer.kind;
            jsonSsrcs.push_back(jsonSsrc);

            jsonSsrc = json::object();
            jsonSsrc["attribute"] = "mslabel";
            jsonSsrc["id"] = encoding.rtx.ssrc;
            jsonSsrc["value"] = OfferMslabel;
            jsonSsrcs.push_back(jsonSsrc);

            jsonSsrc = json::object();
            jsonSsrc["attribute"] = "label";
            jsonSsrc["id"] = encoding.rtx.ssrc;
            jsonSsrc["value"] = OfferMslabel + "-" + consumer.kind;
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
        if (consumer.direction.find("send") != std::string::npos) {
            jsonMedia["direction"] = "sendrecv";
        } else {
            jsonMedia["direction"] = "sendonly";
        }
        // sdp: media: mid
        jsonMedia["mid"] = consumer.rtpParameters.mid;
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
        jsonMedia["type"] = consumer.kind;
        jsonMedia["connection"]["ip"] = "0.0.0.0";
        jsonMedia["connection"]["version"] = 4;
        jsonMedia["icelite"] = "ice-lite";

        mids += consumer.rtpParameters.mid + " ";
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
    jsonSdp["msidSemantic"]["token"] = OfferMslabel;
    jsonSdp["name"] = "-";
    jsonSdp["origin"]["address"] = "0.0.0.0";
    jsonSdp["origin"]["ipVer"] = 4;
    jsonSdp["origin"]["netType"] = "IN";
    jsonSdp["origin"]["sessionId"] = this->rtcTransportParameters.sessionId;
    jsonSdp["origin"]["sessionVersion"] = 2;
    jsonSdp["origin"]["username"] = this->rtcTransportParameters.uesrName;

    jsonSdp["timing"]["start"] = 0;
    jsonSdp["timing"]["stop"] = 0;

    jsonSdp["version"] = 0;

    sdp = sdptransform::write(jsonSdp);

    PMS_DEBUG("SessionId[{}] StreamId[{}] jsonSdp {}", this->sessionId, this->streamId, jsonSdp.dump());
    PMS_DEBUG("SessionId[{}] StreamId[{}] sdp {}", this->sessionId, this->streamId, sdp);

    return 0;
}

int RtcSession::FillAnswer(std::string &sdp)
{
    if (this->jsonIceParameters.is_null() ||
        this->jsonIceCandidateParameters.is_null() ||
        this->jsonDtlsParameters.is_null())
    {
        PMS_ERROR("SessionId[{}] StreamId[{}] Ice dtls parameters needed",
            this->sessionId, this->streamId);
        return -1;
    }

    json jsonSdp = json::object();
    json jsonMedias = json::array();

    std::string mids = "";

    // sdp: media
    for (auto &producer : this->producerParameters) {
        jsonMedias.emplace_back(json::object());
        auto &jsonMedia = jsonMedias[jsonMedias.size() - 1];

        int i = 0;

        // sdp: media: candidates
        if (this->FillCandidates(jsonMedia["candidates"]) != 0) {
            PMS_ERROR("SessionId[{}] StreamId[{}] Fill ice candidate failed",
                this->sessionId, this->streamId);
            return -1;
        }

        // sdp: media: ext
        jsonMedia["ext"] = json::array();
        auto jsonExtIt = jsonMedia.find("ext");
        for (auto &ext : producer.rtpParameters.headerExtensions) {
            jsonExtIt->emplace_back(json::value_t::object);
            (*jsonExtIt)[i]["uri"] = ext.uri;
            (*jsonExtIt)[i]["value"] = ext.id;
            i++;
        }

        // sdp: media: fingerprint
        if (this->rtcTransportParameters.dtlsParameters.fingerprints.size() > 0) {
            for (auto &jsonItem : this->jsonDtlsParameters["fingerprints"]) {
                if (jsonItem["algorithm"].get<std::string>()
                    == this->rtcTransportParameters.dtlsParameters.fingerprints[0].algorithm)
                {
                    jsonMedia["fingerprint"]["type"] = jsonItem["algorithm"].get<std::string>();
                    jsonMedia["fingerprint"]["hash"] = jsonItem["value"].get<std::string>();
                    break;
                }
            }
        }

        // sdp: media: ice
        jsonMedia["icePwd"] = this->jsonIceParameters["password"].get<std::string>();
        jsonMedia["iceUfrag"] = this->jsonIceParameters["usernameFragment"].get<std::string>();

        // sdp: media: setup
        if (this->jsonDtlsParameters["role"] == "server") {
            jsonMedia["setup"] = "passive";
        } else if (this->jsonDtlsParameters["role"] == "client") {
            jsonMedia["setup"] = "active";
        } else if (this->jsonDtlsParameters["role"] == "auto") {
            jsonMedia["setup"] = "active";
        }

        // sdp: media: fmtp & rtcpFb
        jsonMedia["fmtp"] = json::array();
        jsonMedia["rtcpFb"] = json::array();
        auto jsonFmtpIt = jsonMedia.find("fmtp");
        auto jsonRtcpFbIt = jsonMedia.find("rtcpFb");
        std::string payloads = "";
        i = 0;
        for (auto &codec : producer.rtpParameters.codecs) {
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
        for (auto &rtp : producer.rtpParameters.codecs) {
            jsonRtpIt->emplace_back(json::value_t::object);
            auto &jsonRtp = (*jsonRtpIt)[i];
            jsonRtp["payload"] = rtp.payloadType;
            jsonRtp["rate"] = rtp.clockRate;
            jsonRtp["codec"] = RTC::RtpCodecMimeType::subtype2String[rtp.mimeType.subtype];
            if (producer.kind == "audio" && rtp.channels > 0) {
                jsonRtp["encoding"] = rtp.channels;
            }
            i++;
        }

        // sdp: media: ssrcs
        if (producer.direction.find("recv") != std::string::npos) {

            json jsonSsrcs = json::array();
            json jsonGroups = json::array();
            std::string ssrcGroup = "";
            for (auto encoding : producer.rtpParameters.encodings) {
                if (encoding.ssrc == 0) {
                    continue;
                }

                json jsonSsrc = json::object();
                jsonSsrc["attribute"] = "cname";
                jsonSsrc["id"] = encoding.ssrc;
                jsonSsrc["value"] = producer.rtpParameters.rtcp.cname;
                jsonSsrcs.push_back(jsonSsrc);

                jsonSsrc = json::object();
                jsonSsrc["attribute"] = "msid";
                jsonSsrc["id"] = encoding.ssrc;
                jsonSsrc["value"] = OfferMslabel + " " + OfferMslabel + "-" + producer.kind;
                jsonSsrcs.push_back(jsonSsrc);

                jsonSsrc = json::object();
                jsonSsrc["attribute"] = "mslabel";
                jsonSsrc["id"] = encoding.ssrc;
                jsonSsrc["value"] = OfferMslabel;
                jsonSsrcs.push_back(jsonSsrc);

                jsonSsrc = json::object();
                jsonSsrc["attribute"] = "label";
                jsonSsrc["id"] = encoding.ssrc;
                jsonSsrc["value"] = OfferMslabel + "-" + producer.kind;
                jsonSsrcs.push_back(jsonSsrc);

                if (!encoding.hasRtx) {
                    continue;
                }

                jsonSsrc = json::object();
                jsonSsrc["attribute"] = "cname";
                jsonSsrc["id"] = encoding.rtx.ssrc;
                jsonSsrc["value"] = producer.rtpParameters.rtcp.cname;
                jsonSsrcs.push_back(jsonSsrc);

                jsonSsrc = json::object();
                jsonSsrc["attribute"] = "msid";
                jsonSsrc["id"] = encoding.rtx.ssrc;
                jsonSsrc["value"] = OfferMslabel + " " + OfferMslabel + "-" + producer.kind;
                jsonSsrcs.push_back(jsonSsrc);

                jsonSsrc = json::object();
                jsonSsrc["attribute"] = "mslabel";
                jsonSsrc["id"] = encoding.rtx.ssrc;
                jsonSsrc["value"] = OfferMslabel;
                jsonSsrcs.push_back(jsonSsrc);

                jsonSsrc = json::object();
                jsonSsrc["attribute"] = "label";
                jsonSsrc["id"] = encoding.rtx.ssrc;
                jsonSsrc["value"] = OfferMslabel + "-" + producer.kind;
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
        jsonMedia["mid"] = producer.rtpParameters.mid;
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
        jsonMedia["type"] = producer.kind;
        jsonMedia["connection"]["ip"] = "0.0.0.0";
        jsonMedia["connection"]["version"] = 4;
        jsonMedia["icelite"] = "ice-lite";

        mids += producer.rtpParameters.mid + " ";
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
//    jsonSdp["msidSemantic"]["token"] = "*";
    jsonSdp["name"] = "-";
    jsonSdp["origin"]["address"] = "0.0.0.0";
    jsonSdp["origin"]["ipVer"] = 4;
    jsonSdp["origin"]["netType"] = "IN";
    jsonSdp["origin"]["sessionId"] = this->rtcTransportParameters.sessionId;
    jsonSdp["origin"]["sessionVersion"] = 2;
    jsonSdp["origin"]["username"] = this->rtcTransportParameters.uesrName;

    jsonSdp["timing"]["start"] = 0;
    jsonSdp["timing"]["stop"] = 0;

    jsonSdp["version"] = 0;

    sdp = sdptransform::write(jsonSdp);

    PMS_DEBUG("SessionId[{}] StreamId[{}] jsonSdp {}", this->sessionId, this->streamId, jsonSdp.dump());
    PMS_DEBUG("SessionId[{}] StreamId[{}] sdp {}", this->sessionId, this->streamId, sdp);

    return 0;

}

int RtcSession::FillCandidates(json &jsonObject)
{
    jsonObject = json::array();
    for (auto &jsonCandidate : this->jsonIceCandidateParameters) {
        json jsonItem;
        jsonItem["foundation"] = jsonCandidate["foundation"].get<std::string>();
        jsonItem["ip"] = jsonCandidate["ip"].get<std::string>();
        jsonItem["port"] = jsonCandidate["port"].get<int>();
        jsonItem["priority"] = jsonCandidate["priority"].get<int>();
        jsonItem["transport"] = jsonCandidate["protocol"].get<std::string>();
        jsonItem["type"] = jsonCandidate["type"].get<std::string>();
        jsonItem["component"] = 1;
        jsonObject.push_back(jsonItem);
    }

    return 0;
}

void RtcSession::FireEvent(std::string event, json& data)
{
    json jsonRoot = json::object();

    jsonRoot["event"]    = event;
    jsonRoot["data"]     = data;

    for (auto &listener : this->listeners) {
        if (listener) {
            listener->OnRtcSessionEvent(this, jsonRoot);
        }
    }
}

}

