#define PMS_CLASS "pingos::RtcServer"
#define MS_CLASS "pingos::RtcServer"


#include <algorithm>
#include <functional>
#include "Master/Defines.hpp"
#include "Master/RtcServer.hpp"
#include "Master/Log.hpp"
#include "Master/RtcWorker.hpp"
#include "MediaSoupErrors.hpp"
#include "Master/RtcRequest.hpp"
#include "Master/Configuration.hpp"

namespace pingos {

RtcServer::RtcServer(NetServer *netServer, RtcMaster *rtcMaster)
{
    this->Start(netServer, rtcMaster);
}

RtcServer::~RtcServer()
{

}

int RtcServer::Start(NetServer *netServer, RtcMaster *rtcMaster)
{
    if (!netServer) {
        PMS_ERROR("NetServer ptr is nullptr");
        return -1;
    }

    if (!rtcMaster) {
        PMS_ERROR("RtcMaster ptr is nullptr");
        return -1;
    }

    netServer->SetListener(this);

    m_netServer = netServer;
    m_rtcMaster = rtcMaster;

    return 0;
}

void RtcServer::OnRtcSessionAck(RtcSession *rtcSession, json &jsonObject)
{
    std::string sessionId;
    std::string streamId;
    std::string ack;
    std::string reason;
    int error;

    JSON_READ_VALUE_THROW(jsonObject, "sessionId", std::string, sessionId);
    JSON_READ_VALUE_THROW(jsonObject, "streamId", std::string, streamId);
    JSON_READ_VALUE_THROW(jsonObject, "ack", std::string, ack);
    JSON_READ_VALUE_THROW(jsonObject, "error", int, error);
    JSON_READ_VALUE_THROW(jsonObject, "reason", std::string, reason);

    Context *ctx = (Context*) rtcSession->GetContext();

    std::string method = "";
    if (ack == "publish") {
        if (ctx->state == Context::State::READY) {
            return;
        }
        method = "stream.publish";
    } else if (ack == "play") {
        if (ctx->state == Context::State::READY) {
            return;
        }
        method = "stream.play";
    }

    RtcResponse response(ctx->nc, ctx->appId, ctx->uid, ctx->streamId, method);

    if (response.Reply(error, reason.c_str(), jsonObject["data"]) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] Ack[{}], reply failed.",
            sessionId, streamId, ack);
        return;
    }

    ctx->state = Context::State::READY;

    PMS_INFO("SessionId[{}] StreamId[{}] Ack[{}] error[{}] reason[{}] data[{}]",
        sessionId, streamId, ack, error, reason, jsonObject["data"].dump());
}

void RtcServer::OnRtcSessionEvent(RtcSession *rtcSession, json &jsonObject)
{

}

int RtcServer::OnMessage(NetConnection *nc)
{
    std::string reason = "";

    RtcRequest request(nc);
    json jsonObject;

    try {
        jsonObject = json::parse(nc->PopData());
    } catch (const json::parse_error &error) {
        reason = error.what();
        PMS_ERROR("JSON parsing error: {}, message {}", reason, nc->GetData());
        goto _error;
    }

    try {
        request.Parse(jsonObject);
        int ret = 0;
        switch (request.methodId) {
            case RtcRequest::MethodId::STREAM_PUBLISH:
            ret = this->PublishStream(&request);
            break;

            case RtcRequest::MethodId::STREAM_PLAY:
            ret = this->PlayStream(&request);
            break;

            case RtcRequest::MethodId::STREAM_MUTE:
            ret = this->MuteStream(&request);
            break;

            case RtcRequest::MethodId::STREAM_CLOSE:
            ret = this->CloseStream(&request);
            break;

            default:
            break;
        }

        if (ret != 0) {
            PMS_ERROR("StreamId[{}] Request[{}] failed.",
                request.stream, request.method);
            reason = "internal error";
            goto _error;
        }

    } catch (const MediaSoupError& error) {
        reason = error.what();
        PMS_ERROR("discarding wrong rtc message: {}", reason);
        goto _error;
    }

    if (request.count == 0) {
        request.Accept();
    }

    return 0;

_error:
    request.Error(reason.c_str());

    return -1;
}

void RtcServer::OnDisconnect(NetConnection *nc)
{
    RtcSession *rtcSession = (RtcSession *) nc->GetSession();

    if (rtcSession) {
        this->DeleteSession(rtcSession);
    }

    return;
}

std::string& replace_all_distinct(std::string& str, const std::string& old_value, const std::string& new_value)
{
    std::string::size_type pos=0;
    while((pos=str.find(old_value,pos))!= std::string::npos)
    {
        str=str.replace(pos,old_value.length(), new_value);
        if(new_value.length()>0)
        {
            pos += new_value.length();
        }
    }
    return str;
}

int RtcServer::PublishStream(RtcRequest *request)
{
    auto jsonSdpIt = request->jsonData.find("sdp");
    if (jsonSdpIt == request->jsonData.end()) {
        PMS_ERROR("StreamId[{}] Invalid json data, missing sdp", request->stream);
        MS_THROW_ERROR("Missing sdp");
        return -1;
    }

    auto sdp = jsonSdpIt->get<std::string>();

    std::string errorStr = "\\=";
    std::string rightStr = "=";

    sdp = replace_all_distinct(sdp, errorStr, rightStr);

    RtcSession *rtcSession = nullptr;
    rtcSession = CreateRtcSession(request);
    if (!rtcSession) {
        PMS_ERROR("StreamId[{}] create rtc session failed", request->stream);
        MS_THROW_ERROR("Create rtc session failed");
        return -1;
    }

    if (rtcSession->Publish(sdp) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}] publish failed",
            rtcSession->GetSessionId(), request->stream);
        this->DeleteSession(rtcSession);

        MS_THROW_ERROR("publish failed");

        return -1;
    }

    Context *ctx = (Context*) rtcSession->GetContext();

    ctx->state = Context::State::SETUP;

    request->count++;

    return 0;
}

int RtcServer::PlayStream(RtcRequest *request)
{
    auto jsonSdpIt = request->jsonData.find("sdp");
    if (jsonSdpIt == request->jsonData.end()) {
        PMS_ERROR("StreamId[{}] Invalid json data, missing sdp", request->stream);
        MS_THROW_ERROR("Missing sdp");
        return -1;
    }

    auto *worker = FindWorkerByStreamId(request->stream);
    if (worker == nullptr) {
        PMS_ERROR("StreamId[{}] Worker not found", request->stream);

        MS_THROW_ERROR("Worker not found");

        return -1;
    }

    auto publisher = worker->FindPublisher(request->stream);
    if (publisher == nullptr) {
        PMS_ERROR("StreamId[{}] Publisher not found", request->stream);

        MS_THROW_ERROR("Publisher not found");

        return -1;
    }

    auto sdp = jsonSdpIt->get<std::string>();

    RtcSession *rtcSession = nullptr;
    rtcSession = CreateRtcSession(request);
    if (!rtcSession) {
        PMS_ERROR("StreamId[{}] create rtc session failed", request->stream);
        MS_THROW_ERROR("Create rtc session failed");
        return -1;
    }

    rtcSession->SetProducerParameters(*publisher);

    if (rtcSession->Play(sdp)) {
        this->DeleteSession(rtcSession);
        PMS_ERROR("SessionId[{}] StreamId[{}] play failed",
            rtcSession->GetSessionId(), request->stream);
        MS_THROW_ERROR("Play failed");
        return -1;
    }

    request->count++;

    Context *ctx = (Context*) rtcSession->GetContext();

    ctx->state = Context::State::SETUP;

    return 0;
}

int RtcServer::MuteStream(RtcRequest *request)
{
    auto *rtcSession = (RtcSession*) request->nc->GetSession();

    bool muteVideo { false };
    bool muteAudio { false };

    JSON_READ_VALUE_THROW(request->jsonData, "video", bool, muteVideo);
    JSON_READ_VALUE_THROW(request->jsonData, "audio", bool, muteAudio);

    if (muteVideo) {
        rtcSession->Pause("video");
    } else {
        rtcSession->Resume("video");
    }

    if (muteAudio) {
        rtcSession->Pause("audio");
    } else {
        rtcSession->Resume("audio");
    }

    PMS_INFO("SessionId[{}] StreamId[{}] Set mute video[{}] audio[{}]",
        rtcSession->GetSessionId(), request->stream, muteVideo, muteAudio);
    return 0;
}

int RtcServer::CloseStream(RtcRequest *request)
{
    auto *rtcSession = (RtcSession*) request->nc->GetSession();

    rtcSession->Close();
    PMS_INFO("SessionId[{}] StreamId[{}] Closing",
        rtcSession->GetSessionId(), request->stream);

    return 0;
}

RtcWorker* RtcServer::FindWorkerByStreamId(std::string streamId)
{
    if (!m_rtcMaster) {
        PMS_ERROR("StreamId[{}] invalid rtc master, while trying to find worker", streamId);
        return nullptr;
    }

    size_t hashVal = std::hash<std::string>()(streamId);
    size_t slot = hashVal % m_rtcMaster->GetWorkerCount();

    return m_rtcMaster->FindWorker(slot);
}

std::string RtcServer::SpellSessionId()
{
    static uint64_t id = 0;
    std::string sessionId = std::string("sid") + std::to_string(id);

    id++;

    return sessionId;
}

RtcSession* RtcServer::CreateRtcSession(RtcRequest *request)
{
    auto streamId = request->stream;

    auto worker = this->FindWorkerByStreamId(streamId);
    if (!worker) {
        PMS_ERROR("StreamId[{}], Worker not found", streamId);

//        MS_THROW_ERROR("Worker not found");

        return nullptr;
    }

    RtcSession::Role role = RtcSession::Role::NONE;

    if (request->methodId == RtcRequest::MethodId::STREAM_PUBLISH) {
        role = RtcSession::Role::PUBLISHER;
    } else if (request->methodId == RtcRequest::MethodId::STREAM_PLAY) {
        role = RtcSession::Role::PLAYER;
    } else {
        PMS_ERROR("StreamId[{}], a session can only be created on publish or play",
            streamId);

//        MS_THROW_ERROR("A session can only be created on publish or play");

        return nullptr;
    }

    auto sessionId = this->SpellSessionId();
    RtcSession *rtcSession = worker->CreateSession(streamId, sessionId, role);
    if (!rtcSession) {
        PMS_ERROR("SessionId[{}] StreamId[{}] create session failed.", sessionId, streamId);

        return nullptr;
    }

    rtcSession->AddLocalAddress(Configuration::webrtc.listenIp, Configuration::webrtc.announcedIp);

    Context *ctx = new Context();
    ctx->nc       = request->nc;
    ctx->appId    = request->app;
    ctx->uid      = request->uid;
    ctx->streamId = request->stream;

    rtcSession->AddListener(this);
    rtcSession->SetContext(ctx);
    request->nc->SetSession(rtcSession);

    PMS_INFO("SessionId[{}] StreamId[{}], RTC Session creation success, session ptr[{}].",
            sessionId, streamId, (void *)rtcSession);

    return rtcSession;
}

void RtcServer::DeleteSession(RtcSession *session)
{
    auto streamId = session->GetStreamId();

    auto worker = this->FindWorkerByStreamId(streamId);
    if (!worker) {
        PMS_ERROR("StreamId[{}], Worker not found", streamId);
        return;
    }

    PMS_INFO("SessionId[{}] StreamId[{}], RTC Session deleted, session ptr[{}].",
            session->GetSessionId(), streamId, (void *)session);

    Context *ctx = (Context *) session->GetContext();

    if (ctx) {
        session->SetContext(nullptr);

        if (ctx->nc) {
            ctx->nc->SetSession(nullptr);
        }

        delete ctx;
    }

    worker->DeleteSession(streamId, session->GetSessionId());
}

}
