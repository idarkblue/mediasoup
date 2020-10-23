#include "Master/Log.hpp"
#include "Master/RtcSession.hpp"
#include "Master/RtcWorker.hpp"
#include "Channel/Request.hpp"
#include "Master/sdptransform.hpp"

#define PMS_CLASS "pingos::RtcSession"

namespace pingos {

static std::string OfferMslabel = "pingos-rtc-ms-label";

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

uint64_t RtcSession::Request::m_requestId = 0;

RtcSession::Request::Request(const std::string method)
{
    this->Init(method);
}

RtcSession::Request::~Request()
{

}

void RtcSession::Request::Init(const std::string method)
{
    m_id = m_requestId++;
    m_method = method;
    m_jsonInternal = json::object();
    m_jsonData = json::object();

}
void RtcSession::Request::SetInternal(json &jsonObject)
{
    m_jsonInternal = jsonObject;
}

void RtcSession::Request::SetData(json &jsonObject)
{
    m_jsonData = jsonObject;
}

uint64_t RtcSession::Request::GetId()
{
    return m_id;
}

std::string RtcSession::Request::GetMethod()
{
    return m_method;
}

void RtcSession::Request::FillJson(json &jsonObject)
{
    jsonObject["id"] = m_id;
    jsonObject["method"] = m_method;
    jsonObject["internal"] = m_jsonInternal;
    jsonObject["data"] = m_jsonData;
}

std::string RtcSession::Role2String(RtcSession::Role role)
{
    static std::string role2string[] = {
        "none",
        "producer",
        "consumer"
    };

    return role2string[role];
}

RtcSession::RtcSession(Role role, std::string sessionId, std::string stream):
    m_role(role), m_sessionId(sessionId), m_streamId(stream)
{
    m_routerId = "pingos";
    m_transportId = sessionId + std::string("-")
        + Role2String(role) + std::string("-")
        + stream;

    if (role == Role::PUBLISHER) {
        m_videoProducerId = sessionId + std::string("-")
            + Role2String(role) + std::string("-")
            + stream + std::string("-")
            + std::string("video");

        m_audioProducerId = sessionId + std::string("-")
            + Role2String(role) + std::string("-")
            + stream + std::string("-")
            + std::string("audio");
    } else {
        m_videoConsumerId = sessionId + std::string("-")
            + Role2String(role) + std::string("-")
            + stream + std::string("-")
            + std::string("video");

        m_audioConsumerId = sessionId + std::string("-")
            + Role2String(role) + std::string("-")
            + stream + std::string("-")
            + std::string("audio");
    }
}

RtcSession::~RtcSession()
{
    this->Close();
}

void RtcSession::AddListener(RtcSession::Listener *listener)
{
    m_listeners.push_back(listener);
}

void RtcSession::SetWorker(RtcWorker *worker)
{
    m_worker = worker;
    m_status = Status::INITED;
}

RtcSession::Role RtcSession::GetRole()
{
    return m_role;
}

std::string RtcSession::GetSessionId()
{
    return m_sessionId;
}

std::string RtcSession::GetStreamId()
{
    return m_streamId;
}

std::string RtcSession::GetTransportId()
{
    return m_transportId;
}

std::string RtcSession::GetProducerId(std::string kind)
{
    if (kind == "video") {
        return m_videoProducerId;
    } else if (kind == "audio") {
        return m_audioProducerId;
    }

    return "";
}

std::string RtcSession::GetConsumerId(std::string kind)
{
    if (kind == "video") {
        return m_videoConsumerId;
    } else if (kind == "audio") {
        return m_audioConsumerId;
    }

    return "";
}

std::vector<ProducerParameters> &RtcSession::GetProducerParameters()
{
    return m_producerParameters;
}

void RtcSession::ReceiveChannelAck(json &jsonObject)
{
    auto id = jsonObject["id"].get<uint64_t>();
    auto it = m_requestWaittingMap.find(id);

    if (it == m_requestWaittingMap.end()) {
        PMS_ERROR("SessionId[{}] streamId[{}] Invalid ack, unknown request id[{}]",
            m_sessionId, m_streamId, id);
        return;
    }

    auto &request = it->second;
    auto jsonErrorIt = jsonObject.find("error");
    auto jsonAcceptedIt = jsonObject.find("accepted");
    std::string sdp;
    std::string ack;
    switch (Channel::Request::string2MethodId[request.GetMethod()]) {
        case Channel::Request::MethodId::ROUTER_CREATE_WEBRTC_TRANSPORT:
        if (jsonAcceptedIt != jsonObject.end()) {
            m_jsonIceParameters = jsonObject["data"]["iceParameters"];
            m_jsonIceCandidateParameters = jsonObject["data"]["iceCandidates"];
            m_jsonDtlsParameters = jsonObject["data"]["dtlsParameters"];
        }
//        case Channel::Request::MethodId::TRANSPORT_CONNECT:
//        case Channel::Request::MethodId::TRANSPORT_PRODUCE:
//        case Channel::Request::MethodId::TRANSPORT_CONSUME:
        if (m_role == Role::PUBLISHER) {
            ack = "publish";
            if (this->FillAnswer(sdp) != 0) {
                jsonObject["error"] = 1;
                jsonObject["reason"] = "create answer sdp failed";
                jsonErrorIt = jsonObject.find("error");
            }
        } else if (m_role == Role::PLAYER) {
            ack = "play";
            if (this->FillOffer(sdp) != 0) {
                jsonObject["error"] = 1;
                jsonObject["reason"] = "create answer sdp failed";
                jsonErrorIt = jsonObject.find("error");
            }
        }

        jsonObject["data"]["sdp"] = sdp;

        break;

        default:
        PMS_INFO("SessionId[{}] StreamId[{}] ignore ack[{}] response",
            m_sessionId, m_streamId, request.GetMethod());
        return;
        break;
    }

    json jsonAck = json::object();
    jsonAck["sessionId"] = m_sessionId;
    jsonAck["streamId"] = m_streamId;
    jsonAck["ack"] = ack;
    if (jsonErrorIt != jsonObject.end()) {
        std::string reason;
        jsonAck["error"] = 1;
        JSON_READ_VALUE_DEFAULT(jsonObject, "reason", std::string, reason, "");
        jsonAck["reason"] = reason;
    } else {
        jsonAck["error"] = 0;
        jsonAck["reason"] = "OK";
        jsonAck["data"] = jsonObject["data"];
        m_status = Status::TRANSPORT_CREATED;
    }

    for (auto listener : m_listeners) {
        listener->OnRtcSessionAck(this, jsonAck);
    }
}

void RtcSession::ReceiveChannelEvent(json &jsonObject)
{
    PMS_INFO("SessionId[{}] streamId[{}] receive event[{}]",
        m_sessionId, m_streamId, jsonObject.dump());

    jsonObject["sessionId"] = m_sessionId;
    jsonObject["streamId"] = m_streamId;

    for (auto listener : m_listeners) {
        listener->OnRtcSessionEvent(this, jsonObject);
    }
}

int RtcSession::Publish(std::string sdp)
{
    SdpInfo si(sdp);

    if (si.TransformSdp(m_rtcTransportParameters, m_producerParameters) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] publish failed, Transform sdp error",
            m_sessionId, m_streamId);
        return -1;
    }

    Request request;
    if (GenerateRouterRequest("worker.createRouter", request) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] publish failed, generate router request error",
            m_sessionId, m_streamId);
        return -1;
    }

    if (ActiveRtcSessionRequest(request) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] publish failed, run router request error",
            m_sessionId, m_streamId);
        return -1;
    }

    if (GenerateWebRtcTransportRequest("router.createWebRtcTransport", request) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] publish failed, generate webrtctransport request error",
            m_sessionId, m_streamId);
        return -1;
    }

    if (ActiveRtcSessionRequest(request) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] publish failed, run webrtctransport request error",
            m_sessionId, m_streamId);
        return -1;
    }

    if (GenerateWebRtcTransportRequest("transport.connect", request) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] publish failed, transport connect request error",
            m_sessionId, m_streamId);
        return -1;
    }

    if (ActiveRtcSessionRequest(request) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] publish failed, run transport connect request error",
            m_sessionId, m_streamId);
        return -1;
    }

    for (auto &producer : m_producerParameters) {
        if (GenerateProducerRequest("transport.produce", producer.kind, request) != 0) {
            PMS_ERROR("SessionId[{}] StreamId[{}] publish failed, generate producer[{}] request error",
                m_sessionId, m_streamId, producer.kind);
            return -1;
        }
        if (ActiveRtcSessionRequest(request) != 0) {
            PMS_ERROR("SessionId[{}] StreamId[{}] publish failed, run producer[{}] request error",
                m_sessionId, m_streamId, producer.kind);
            return -1;
        }
    }

    PMS_INFO("SessionId[{}] StreamId[{}] publish success", m_sessionId, m_streamId);

    return 0;
}

int RtcSession::Play(std::string sdp)
{
    SdpInfo si(sdp);

    if (si.TransformSdp(m_rtcTransportParameters) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] play failed, Transform sdp error",
            m_sessionId, m_streamId);
        return -1;
    }

    Request request;
    if (GenerateRouterRequest("worker.createRouter", request) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] play failed, generate router request error",
            m_sessionId, m_streamId);
        return -1;
    }

    if (ActiveRtcSessionRequest(request) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] play failed, run router request error",
            m_sessionId, m_streamId);
        return -1;
    }

    if (GenerateWebRtcTransportRequest("router.createWebRtcTransport", request) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] play failed, generate webrtctransport request error",
            m_sessionId, m_streamId);
        return -1;
    }

    if (ActiveRtcSessionRequest(request) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] play failed, run webrtctransport request error",
            m_sessionId, m_streamId);
        return -1;
    }

    if (GenerateWebRtcTransportRequest("transport.connect", request) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] play failed, transport connect request error",
            m_sessionId, m_streamId);
        return -1;
    }

    if (ActiveRtcSessionRequest(request) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] play failed, run transport connect request error",
            m_sessionId, m_streamId);
        return -1;
    }

    for (auto &consumer : m_consumerParameters) {
        if (GenerateConsumerRequest("transport.consume", consumer.kind, request) != 0) {
            PMS_ERROR("SessionId[{}] StreamId[{}] play failed, generate consumer[{}] request error",
                m_sessionId, m_streamId, consumer.kind);
            return -1;
        }
        if (ActiveRtcSessionRequest(request) != 0) {
            PMS_ERROR("SessionId[{}] StreamId[{}] play failed, consumer[{}] run request error",
                m_sessionId, m_streamId, consumer.kind);
            return -1;
        }
    }

    PMS_INFO("SessionId[{}] StreamId[{}] play success", m_sessionId, m_streamId);

    return 0;
}

int RtcSession::Pause(std::string kind)
{
    Request request;

    for (auto &consumer : m_consumerParameters) {
        if (kind != consumer.kind) {
            continue;
        }

        if (GenerateConsumerRequest("consume.pause", consumer.kind, request) != 0) {
            PMS_ERROR("SessionId[{}] StreamId[{}] pause failed, generate consumer[{}] request error",
                m_sessionId, m_streamId, consumer.kind);
            return -1;
        }

        if (ActiveRtcSessionRequest(request) != 0) {
            PMS_ERROR("SessionId[{}] StreamId[{}] pause failed, consumer[{}] run request error",
                m_sessionId, m_streamId, consumer.kind);
            return -1;
        }
    }

    return 0;
}

int RtcSession::Resume(std::string kind)
{
    Request request;

    for (auto &consumer : m_consumerParameters) {
        if (kind != consumer.kind) {
            continue;
        }

        if (GenerateConsumerRequest("consume.resume", consumer.kind, request) != 0) {
            PMS_ERROR("SessionId[{}] StreamId[{}] resume failed, generate consumer[{}] request error",
                m_sessionId, m_streamId, consumer.kind);
            return -1;
        }

        if (ActiveRtcSessionRequest(request) != 0) {
            PMS_ERROR("SessionId[{}] StreamId[{}] resume failed, consumer[{}] run request error",
                m_sessionId, m_streamId, consumer.kind);
            return -1;
        }
    }

    return 0;
}

int RtcSession::Close()
{
    if (m_status == Status::CLOSED) {
        PMS_WARN("SessionId[{}] StreamId[{}] the session has been closed",
            m_sessionId, m_streamId);
        return 0;
    }

    Request request;
    if (GenerateWebRtcTransportRequest("transport.close", request) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] close failed, GenerateWebRtcTransportRequest error",
            m_sessionId, m_streamId);
        return -1;
    }

    if (ActiveRtcSessionRequest(request) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] close failed, run request error",
            m_sessionId, m_streamId);
        return -1;
    }

    m_status = Status::CLOSED;

    return 0;
}

int RtcSession::GetLocalSdp(std::string &sdp)
{
    return 0;
}

void RtcSession::AddLocalAddress(std::string ip, std::string announcedIp)
{
    m_rtcTransportParameters.AddLocalAddr(ip, announcedIp);
}

int RtcSession::SetProducerParameters(RtcSession &rtcSession)
{
    std::vector<ProducerParameters> &producerParameters = rtcSession.GetProducerParameters();

    for (auto &producer : producerParameters) {
        m_consumerParameters.emplace_back();
        auto &consumer = m_consumerParameters.back();
        if (consumer.SetRtpParameters(producer) != 0) {
            PMS_ERROR("SessionId[{}] StreamId[{}] Set consumer's rtp parameters failed",
                m_sessionId, m_streamId);
            return -1;
        }
    }

    m_videoProducerId = rtcSession.GetProducerId("video");
    m_audioProducerId = rtcSession.GetProducerId("audio");

    return 0;
}

void RtcSession::SetContext(void *ctx)
{
    m_ctx = ctx;
}

void* RtcSession::GetContext()
{
    return m_ctx;
}

int RtcSession::GenerateRouterRequest(std::string method, Request &request)
{
    request.Init(method);

    if (Channel::Request::string2MethodId.count(method) == 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] Invalid method, unknown method {}",
            m_sessionId, m_streamId, method);
        return -1;
    }

    json jsonInternal = json::object();
    json jsonData = json::object();

    jsonInternal["routerId"] = this->m_routerId;

    switch (Channel::Request::string2MethodId[method]) {
        case Channel::Request::MethodId::WORKER_CREATE_ROUTER:
        break;
        case Channel::Request::MethodId::ROUTER_DUMP:
        break;
        case Channel::Request::MethodId::ROUTER_CLOSE:
        break;
        default:
        PMS_ERROR("SessionId[{}] StreamId[{}] Invalid method, method {} not supportted",
            m_sessionId, m_streamId, method);
        return -1;
    }

    PMS_DEBUG("internal {}", jsonInternal.dump());

    request.SetInternal(jsonInternal);
    request.SetData(jsonData);

    return 0;
}

int RtcSession::GenerateWebRtcTransportRequest(std::string method, Request &request)
{
    request.Init(method);

    if (Channel::Request::string2MethodId.count(method) == 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] Invalid method, unknown method {}",
            m_sessionId, m_streamId, method);
        return -1;
    }
    json jsonInternal = json::object();
    json jsonData = json::object();

    jsonInternal["routerId"] = this->m_routerId;
    jsonInternal["transportId"] = this->m_transportId;

    switch (Channel::Request::string2MethodId[method]) {
        case Channel::Request::MethodId::ROUTER_CREATE_WEBRTC_TRANSPORT:
        m_rtcTransportParameters.FillJsonTransport(jsonData);
        break;

        case Channel::Request::MethodId::TRANSPORT_CONNECT:
        m_rtcTransportParameters.FillJsonDtls(jsonData);
        break;

        case Channel::Request::MethodId::TRANSPORT_GET_STATS:
        case Channel::Request::MethodId::TRANSPORT_DUMP:
        case Channel::Request::MethodId::TRANSPORT_CLOSE:
        break;

        default:
        PMS_ERROR("SessionId[{}] StreamId[{}] Invalid method, method {} not supportted",
            m_sessionId, m_streamId, method);
        return -1;
    }

    request.SetInternal(jsonInternal);
    request.SetData(jsonData);

    return 0;
}

int RtcSession::GenerateProducerRequest(std::string method, std::string kind, Request &request)
{
    request.Init(method);

    if (Channel::Request::string2MethodId.count(method) == 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] Invalid method, unknown method {}",
            m_sessionId, m_streamId, method);
        return -1;
    }

    std::string producerId = "";
    if (kind == "video") {
        producerId = m_videoProducerId;
    } else if (kind == "audio") {
        producerId = m_audioProducerId;
    } else {
        PMS_ERROR("SessionId[{}] StreamId[{}] Invalid kind {}", m_sessionId, m_streamId, kind);
        return -1;
    }

    json jsonInternal = json::object();
    json jsonData = json::object();

    jsonInternal["routerId"] = this->m_routerId;
    jsonInternal["transportId"] = this->m_transportId;
    jsonInternal["producerId"] = producerId;

    switch (Channel::Request::string2MethodId[method]) {
        case Channel::Request::MethodId::TRANSPORT_PRODUCE:
        for (auto &producer : m_producerParameters) {
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
            m_sessionId, m_streamId, method);
        return -1;
    }

    request.SetInternal(jsonInternal);
    request.SetData(jsonData);

    return 0;
}

int RtcSession::GenerateConsumerRequest(std::string method, std::string kind, Request &request)
{
    request.Init(method);

    if (Channel::Request::string2MethodId.count(method) == 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] Invalid method, unknown method {}",
            m_sessionId, m_streamId, method);
        return -1;
    }

    std::string consumerId = "";
    std::string producerId = "";

    if (kind == "video") {
        consumerId = m_videoConsumerId;
        producerId = m_videoProducerId;
    } else if (kind == "audio") {
        consumerId = m_audioConsumerId;
        producerId = m_audioProducerId;
    } else {
        PMS_ERROR("SessionId[{}] StreamId[{}] Invalid kind {}", m_sessionId, m_streamId, kind);
        return -1;
    }

    json jsonInternal = json::object();
    json jsonData = json::object();

    jsonInternal["routerId"] = this->m_routerId;
    jsonInternal["transportId"] = this->m_transportId;
    jsonInternal["consumerId"] = consumerId;
    jsonInternal["producerId"] = producerId;

    switch (Channel::Request::string2MethodId[method]) {
        case Channel::Request::MethodId::TRANSPORT_CONSUME:
        for (auto &consumer : m_consumerParameters) {
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
            m_sessionId, m_streamId, method);
        return -1;
    }

    request.SetInternal(jsonInternal);
    request.SetData(jsonData);

    return 0;
}

int RtcSession::ActiveRtcSessionRequest(Request &request)
{
    if (m_worker->SendRequest(this, request) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] run request failed",
            m_sessionId, m_streamId);

        return -1;
    }

    m_requestWaittingMap[request.GetId()] = request;

    return 0;
}

int RtcSession::FillOffer(std::string &sdp)
{
    if (m_jsonIceParameters.is_null() ||
        m_jsonIceCandidateParameters.is_null() ||
        m_jsonDtlsParameters.is_null())
    {
        PMS_ERROR("SessionId[{}] StreamId[{}] Ice dtls parameters needed",
            m_sessionId, m_streamId);
        return -1;
    }

    json jsonSdp = json::object();
    json jsonMedias = json::array();

    std::string mids = "";

    // sdp: media
    for (auto &consumer : m_consumerParameters) {
        jsonMedias.emplace_back(json::object());
        auto &jsonMedia = jsonMedias[jsonMedias.size() - 1];

        int i = 0;

        // sdp: media: candidates
        if (this->FillCandidates(jsonMedia["candidates"]) != 0) {
            PMS_ERROR("SessionId[{}] StreamId[{}] Fill ice candidate failed",
                m_sessionId, m_streamId);
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
        if (m_rtcTransportParameters.dtlsParameters.fingerprints.size() > 0) {
            for (auto &jsonItem : m_jsonDtlsParameters["fingerprints"]) {
                if (jsonItem["algorithm"].get<std::string>()
                    == m_rtcTransportParameters.dtlsParameters.fingerprints[0].algorithm)
                {
                    jsonMedia["fingerprint"]["type"] = jsonItem["algorithm"].get<std::string>();
                    jsonMedia["fingerprint"]["hash"] = jsonItem["value"].get<std::string>();
                    break;
                }
            }
        }

        // sdp: media: ice
        jsonMedia["icePwd"] = m_jsonIceParameters["password"].get<std::string>();
        jsonMedia["iceUfrag"] = m_jsonIceParameters["usernameFragment"].get<std::string>();

        // sdp: media: setup
        if (m_jsonDtlsParameters["role"] == "server") {
            jsonMedia["setup"] = "passive";
        } else if (m_jsonDtlsParameters["role"] == "client") {
            jsonMedia["setup"] = "active";
        } else if (m_jsonDtlsParameters["role"] == "auto") {
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
        jsonMedia["direction"] = "sendonly";
        // sdp: media: mid
        jsonMedia["mid"] = consumer.rtpParameters.mid;
        // sdp: media: port (ignore)
        jsonMedia["port"] = 9;
        // sdp: media: protocol
        jsonMedia["protocol"] = "RTP/SAVPF";
        // sdp: media: rtcpMux
        jsonMedia["rtcpMux"] = "rtcp-mux";
        // sdp: media: rtcpRsize
        jsonMedia["rtcpRsize"] = "rtcp-rsize";
        // sdp: media: x-google-flag
        jsonMedia["xGoogleFlag"] = "conference";
        // sdp: media: type (audio or video)
        jsonMedia["type"] = consumer.kind;

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

    jsonSdp["connection"]["ip"] = "127.0.0.1";
    jsonSdp["connection"]["version"] = 4;
    jsonSdp["icelite"] = "ice-lite";
    jsonSdp["msidSemantic"]["semantic"] = "WMS";
    jsonSdp["msidSemantic"]["token"] = OfferMslabel;
    jsonSdp["name"] = "-";
    jsonSdp["origin"]["address"] = "0.0.0.0";
    jsonSdp["origin"]["ipVer"] = 4;
    jsonSdp["origin"]["netType"] = "IN";
    jsonSdp["origin"]["sessionId"] =  std::time(nullptr);
    jsonSdp["origin"]["sessionVersion"] = 2;
    jsonSdp["origin"]["username"] = "pingos-rtc";

    jsonSdp["timing"]["start"] = 0;
    jsonSdp["timing"]["stop"] = 0;

    jsonSdp["version"] = 0;

    sdp = sdptransform::write(jsonSdp);

    PMS_DEBUG("SessionId[{}] StreamId[{}] jsonSdp {}", m_sessionId, m_streamId, jsonSdp.dump());
    PMS_DEBUG("SessionId[{}] StreamId[{}] sdp {}", m_sessionId, m_streamId, sdp);

    return 0;
}

int RtcSession::FillAnswer(std::string &sdp)
{
    if (m_jsonIceParameters.is_null() ||
        m_jsonIceCandidateParameters.is_null() ||
        m_jsonDtlsParameters.is_null())
    {
        PMS_ERROR("SessionId[{}] StreamId[{}] Ice dtls parameters needed",
            m_sessionId, m_streamId);
        return -1;
    }

    json jsonSdp = json::object();
    json jsonMedias = json::array();

    std::string mids = "";

    // sdp: media
    for (auto &producer : m_producerParameters) {
        jsonMedias.emplace_back(json::object());
        auto &jsonMedia = jsonMedias[jsonMedias.size() - 1];

        int i = 0;

        // sdp: media: candidates
        if (this->FillCandidates(jsonMedia["candidates"]) != 0) {
            PMS_ERROR("SessionId[{}] StreamId[{}] Fill ice candidate failed",
                m_sessionId, m_streamId);
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
        if (m_rtcTransportParameters.dtlsParameters.fingerprints.size() > 0) {
            for (auto &jsonItem : m_jsonDtlsParameters["fingerprints"]) {
                if (jsonItem["algorithm"].get<std::string>()
                    == m_rtcTransportParameters.dtlsParameters.fingerprints[0].algorithm)
                {
                    jsonMedia["fingerprint"]["type"] = jsonItem["algorithm"].get<std::string>();
                    jsonMedia["fingerprint"]["hash"] = jsonItem["value"].get<std::string>();
                    break;
                }
            }
        }

        // sdp: media: ice
        jsonMedia["icePwd"] = m_jsonIceParameters["password"].get<std::string>();
        jsonMedia["iceUfrag"] = m_jsonIceParameters["usernameFragment"].get<std::string>();

        // sdp: media: setup
        if (m_jsonDtlsParameters["role"] == "server") {
            jsonMedia["setup"] = "passive";
        } else if (m_jsonDtlsParameters["role"] == "client") {
            jsonMedia["setup"] = "active";
        } else if (m_jsonDtlsParameters["role"] == "auto") {
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

        // sdp: media: direction
        jsonMedia["direction"] = "sendrecv";
        // sdp: media: mid
        jsonMedia["mid"] = producer.rtpParameters.mid;
        // sdp: media: port (ignore)
        jsonMedia["port"] = 9;
        // sdp: media: protocol
        jsonMedia["protocol"] = "RTP/SAVPF";
        // sdp: media: rtcpMux
        jsonMedia["rtcpMux"] = "rtcp-mux";
        // sdp: media: rtcpRsize
        jsonMedia["rtcpRsize"] = "rtcp-rsize";
        // sdp: media: x-google-flag
        jsonMedia["xGoogleFlag"] = "conference";
        // sdp: media: type (audio or video)
        jsonMedia["type"] = producer.kind;

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

    jsonSdp["connection"]["ip"] = "127.0.0.1";
    jsonSdp["connection"]["version"] = 4;
    jsonSdp["icelite"] = "ice-lite";
    jsonSdp["msidSemantic"]["semantic"] = "WMS";
    jsonSdp["msidSemantic"]["token"] = "*";
    jsonSdp["name"] = "-";
    jsonSdp["origin"]["address"] = "0.0.0.0";
    jsonSdp["origin"]["ipVer"] = 4;
    jsonSdp["origin"]["netType"] = "IN";
    jsonSdp["origin"]["sessionId"] =  std::time(nullptr);
    jsonSdp["origin"]["sessionVersion"] = 2;
    jsonSdp["origin"]["username"] = "pingos-rtc";

    jsonSdp["timing"]["start"] = 0;
    jsonSdp["timing"]["stop"] = 0;

    jsonSdp["version"] = 0;

    sdp = sdptransform::write(jsonSdp);

    PMS_DEBUG("SessionId[{}] StreamId[{}] jsonSdp {}", m_sessionId, m_streamId, jsonSdp.dump());
    PMS_DEBUG("SessionId[{}] StreamId[{}] sdp {}", m_sessionId, m_streamId, sdp);

    return 0;

}

int RtcSession::FillCandidates(json &jsonObject)
{
    jsonObject = json::array();
    for (auto &jsonCandidate : m_jsonIceCandidateParameters) {
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

}
