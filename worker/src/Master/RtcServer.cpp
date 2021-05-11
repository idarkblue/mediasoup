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

RtcServer::RtcServer(RtcMaster *rtcMaster)
{
    this->SetMaster(rtcMaster);
}

RtcServer::~RtcServer()
{

}

int RtcServer::SetMaster(RtcMaster *rtcMaster)
{
    if (!rtcMaster) {
        PMS_ERROR("RtcMaster ptr is nullptr");
        return -1;
    }

    this->rtcMaster = rtcMaster;

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

    if (ctx->state == Context::State::READY) {
        return;
    }

    std::string method = "";
    if (ack == "publish") {
        method = "stream.publish";
    } else if (ack == "play") {
        method = "stream.play";
    }

    RtcResponse response(ctx->nc, rtcSession->GetSessionId(), ctx->streamId, method);

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

void RtcServer::OnRtcSessionClosed(RtcSession *session)
{
    PMS_INFO("SessionId[{}] StreamId[{}] rtc session closed.", session->GetSessionId(), session->GetStreamId());

    auto streamId = session->GetStreamId();

    Context *ctx = (Context *) session->GetContext();

    if (ctx) {
        session->SetContext(nullptr);

        if (ctx->nc) {
            ctx->nc->SetSession(nullptr);
            auto it = this->waittingPlayRequestMap.find(session->GetStreamId());
            if (it != this->waittingPlayRequestMap.end()) {
                for (auto requestIt = it->second.begin(); requestIt != it->second.end(); ++requestIt) {
                    auto request = *requestIt;
                    if (request->nc == ctx->nc) {
                        it->second.erase(requestIt);
                        break;
                    }
                }
            }
        }

        delete ctx;
    }

    return;
}

int RtcServer::OnMessage(NetConnection *nc)
{
    std::string reason = "";

    RtcRequest *request = new RtcRequest(nc);
    json jsonObject;

    try {
        std::string message { "{}" };
        nc->PopData(message);
        jsonObject = json::parse(message);
    } catch (const json::parse_error &error) {
        reason = error.what();
        PMS_ERROR("JSON parsing error: {}, message {}", reason, nc->GetData());
        goto _error;
    }

    try {
        request->Parse(jsonObject);
        int ret = 0;
        switch (request->methodId) {
            case RtcRequest::MethodId::SESSION_SETUP:
            ret = this->SetupSession(request);
            break;

            case RtcRequest::MethodId::STREAM_PUBLISH:
            ret = this->PublishStream(request);
            break;

            case RtcRequest::MethodId::STREAM_PLAY:
            ret = this->PlayStream(request);
            break;

            case RtcRequest::MethodId::STREAM_MUTE:
            ret = this->MuteStream(request);
            break;

            case RtcRequest::MethodId::STREAM_CLOSE:
            ret = this->CloseStream(request);
            break;

            case RtcRequest::MethodId::STREAM_HEARTBEAT:
            ret = this->Heartbeat(request);
            break;

            case RtcRequest::MethodId::STREAM_SHUTDOWN:
            ret = this->ShutdownStream(request);
            break;

            default:
            ret = -1;
            reason = "invalid message";
            break;
        }

        if (ret != 0) {
            PMS_ERROR("StreamId[{}] Request[{}] failed.",
                request->stream, request->method);
            reason = "internal error";
            goto _error;
        }

    } catch (const MediaSoupError& error) {
        reason = error.what();
        PMS_ERROR("discarding wrong rtc message: {}", reason);
        goto _error;
    }

    if (request->count == 0) {
        request->Accept();
    }

    if (request->count == 0) {
        delete request;
    }

    return 0;

_error:
    request->Error(reason.c_str());

    if (request->count == 0) {
        delete request;
    }

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

int RtcServer::SetupSession(RtcRequest *request)
{
    auto jsonRoleIt = request->jsonData.find("role");
    if (jsonRoleIt == request->jsonData.end()) {
        MS_THROW_ERROR("Missing role");
        return -1;
    }

    std::string role = jsonRoleIt->get<std::string>();

    RtcSession *rtcSession = nullptr;

    RtcSession::Role enRole = RtcSession::String2Role(role);
    if (enRole == RtcSession::Role::NONE) {
        MS_THROW_ERROR("Unknown role %s", role.c_str());
        return -1;
    }

    rtcSession = CreateSession(request->nc, request->stream, enRole, false);
    if (rtcSession == nullptr) {
        MS_THROW_ERROR("StreamId[%s] create session failed", request->stream.c_str());
        return -1;
    }

    PMS_INFO("SessionId[{}] StreamId[{}] Setup Session success", rtcSession->GetSessionId(), request->stream);

    return 0;
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

    RtcSession *rtcSession = nullptr;
    if (request->session.empty()) {
        rtcSession = this->CreateSession(request->nc, request->stream, RtcSession::Role::PUBLISHER, true);
    } else {
        rtcSession = FindSession(request);
    }

    if (!rtcSession) {
        PMS_ERROR("StreamId[{}] create rtc session failed", request->stream);
        MS_THROW_ERROR("Create rtc session failed");
        return -1;
    }

    if (rtcSession->GetRole() != RtcSession::Role::PUBLISHER) {
        PMS_ERROR("SessionId[{}] StreamId[{}] role[{}] publish not allowed",
            rtcSession->GetSessionId(), request->stream, rtcSession->GetRole());
        MS_THROW_ERROR("publish not allowed");
        return -1;
    }

    try {
        if (rtcSession->Publish(sdp) != 0) {
            PMS_ERROR("SessionId[{}] StreamId[{}] publish failed",
                rtcSession->GetSessionId(), request->stream);
            this->DeleteSession(rtcSession);

            MS_THROW_ERROR("publish failed");

            return -1;
        }
    } catch (const MediaSoupError& error) {
        this->DeleteSession(rtcSession);

        MS_THROW_ERROR("publish failed, %s", error.what());
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

    std::string transport;
    JSON_READ_VALUE_DEFAULT(request->jsonData, "transport", std::string, transport, "webrtcTransport");

    auto *publisher = this->rtcMaster->FindPublisher(request->stream);
    if (publisher == nullptr) {
        if (Configuration::pull.servers.size() != 0) {
            request->count++;
            this->Pull(request);
            this->HoldPlayRequest(request);
            PMS_INFO("StreamId[{}] waritting for pulling, hold the request.", request->stream);
            return 0;
        }

        PMS_ERROR("StreamId[{}] Publisher not found", request->stream);

        MS_THROW_ERROR("StreamId[%s] Publisher not found", request->stream.c_str());

        return -1;
    }

    auto sdp = jsonSdpIt->get<std::string>();

    RtcSession *rtcSession = nullptr;

    if (request->session.empty()) {
        rtcSession = CreateSession(request->nc, request->stream, RtcSession::Role::PLAYER, true);
    } else {
        rtcSession = FindSession(request);
    }

    if (!rtcSession) {
        PMS_ERROR("StreamId[{}] create rtc session failed", request->stream);
        MS_THROW_ERROR("Create rtc session failed");
        return -1;
    }

    if (rtcSession->GetRole() != RtcSession::Role::PLAYER) {
        PMS_ERROR("SessionId[{}] StreamId[{}] role[{}] play not allowed",
            rtcSession->GetSessionId(), request->stream, rtcSession->GetRole());
        MS_THROW_ERROR("play not allowed");
        return -1;
    }

    rtcSession->SetProducerParameters(*publisher);

    try {
        if (rtcSession->Play(sdp)) {
            this->DeleteSession(rtcSession);
            PMS_ERROR("SessionId[{}] StreamId[{}] play failed",
                rtcSession->GetSessionId(), request->stream);
            MS_THROW_ERROR("Play failed");
            return -1;
        }
    } catch (const MediaSoupError& error) {
        this->DeleteSession(rtcSession);

        MS_THROW_ERROR("play failed, %s", error.what());
    }

    Context *ctx = (Context*) rtcSession->GetContext();

    ctx->state = Context::State::SETUP;

    request->count++;

    return 0;
}

int RtcServer::MuteStream(RtcRequest *request)
{
    auto *rtcSession = this->FindSession(request);

    if (!rtcSession) {
        MS_THROW_ERROR("Session not found.");

        return -1;
    }

    if (rtcSession->GetRole() != RtcSession::Role::PUBLISHER &&
        rtcSession->GetRole() != RtcSession::Role::PLAYER)
    {
        PMS_ERROR("SessionId[{}] StreamId[{}] role[{}] mute not allowed",
            rtcSession->GetSessionId(), request->stream, rtcSession->GetRole());
        MS_THROW_ERROR("mute not allowed");
        return -1;
    }

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
    auto *rtcSession = this->FindSession(request);

    if (!rtcSession) {
        MS_THROW_ERROR("Session not found.");

        return -1;
    }

    if (!rtcSession) {
        PMS_INFO("SessionId[{}] StreamId[{}] session not found",
            request->session, request->stream);
        return -1;
    }

    rtcSession->Close();

    PMS_INFO("SessionId[{}] StreamId[{}] Closing",
        rtcSession->GetSessionId(), request->stream);

    return 0;
}

int RtcServer::Heartbeat(RtcRequest *request)
{
    return 0;
}

int RtcServer::ShutdownStream(RtcRequest *request)
{
    auto *rtcSession = this->FindSession(request);

    if (!rtcSession) {
        MS_THROW_ERROR("Session not found.");

        return -1;
    }

    if (!rtcSession) {
        PMS_INFO("SessionId[{}] StreamId[{}] session not found",
            request->session, request->stream);
        return -1;
    }

    rtcSession->Close();

    PMS_INFO("SessionId[{}] StreamId[{}] shutting down",
        rtcSession->GetSessionId(), request->stream);

    return 0;
}

int RtcServer::Pull(RtcRequest *request)
{
    auto rtcSession = this->rtcMaster->CreateSession(request->stream, RtcSession::Role::PUBLISHER);

    for (auto &server : Configuration::pull.servers) {
        RtspClient *rtspClient = new RtspClient(this->rtcMaster, rtcSession);
        rtspClient->SetListener(this);
        rtspClient->SetRtcSession(rtcSession);
        std::string url = std::string("rtsp://") + server.ip +
                        std::string(":") + std::to_string(server.port) +
                        std::string("/") + request->stream;
        rtspClient->Play(url);
    }

    return 0;
}

void RtcServer::HoldPlayRequest(RtcRequest *request)
{
    auto it = this->waittingPlayRequestMap.find(request->stream);

    if (it == this->waittingPlayRequestMap.end()) {
        std::list<RtcRequest*> requests;
        requests.push_back(request);
        this->waittingPlayRequestMap[request->stream] = requests;
    } else {
        auto &requests = it->second;
        requests.push_back(request);
    }
}

void RtcServer::ActivePlayRequest(std::string streamId)
{
    auto it = this->waittingPlayRequestMap.find(streamId);
    if (it == this->waittingPlayRequestMap.end()) {
        return;
    }

    PMS_INFO("StreamId[{}] Active players", streamId);

    for (auto request : it->second) {
        this->PlayStream(request);
    }

}

void RtcServer::OnRtspClientPlayCompleted(RtspClient *client)
{
    auto rtcSession = client->GetRtcSession();
    PMS_INFO("StreamId[{}] rtsp client[{}] play completed.",
        rtcSession->GetStreamId(), client->GetURL());

    this->ActivePlayRequest(rtcSession->GetStreamId());
}

void RtcServer::OnRtspClientPublishCompleted(RtspClient *client)
{
}

void RtcServer::OnRtspClientError(RtspClient *client, RtspClient::RtspClientError errorCode, RtspReplyCode rtspCode)
{
    auto *rtcSession = client->GetRtcSession();

    if (rtcSession) {
        PMS_INFO("StreamId[{}] rtsp client[{}] error code {}",
            rtcSession->GetStreamId(), client->GetURL(), errorCode);
    }

//    this->DeleteSession(client->GetRtcSession());
}

RtcSession* RtcServer::CreateSession(NetConnection *nc, std::string streamId, RtcSession::Role role, bool attach)
{
    auto worker = this->rtcMaster->FindWorker(streamId);
    if (!worker) {
        PMS_ERROR("StreamId[{}], Worker not found", streamId);

        MS_THROW_ERROR("Worker not found");

        return nullptr;
    }

    RtcSession *rtcSession = this->rtcMaster->CreateSession(streamId, role);

    if (!rtcSession) {
        PMS_ERROR("StreamId[{}] create session failed.", streamId);

        return nullptr;
    }

    auto sessionId = rtcSession->GetSessionId();

    rtcSession->AddLocalAddress(Configuration::webrtc.listenIp, Configuration::webrtc.announcedIp);

    Context *ctx = new Context();
    ctx->nc       = nc;
    ctx->streamId = streamId;

    rtcSession->AddListener(this);
    rtcSession->SetContext(ctx);
    if (attach) {
        nc->SetSession(rtcSession);
    }

    PMS_INFO("SessionId[{}] StreamId[{}], RTC Session creation success, session ptr[{}].",
            sessionId, streamId, (void *)rtcSession);

    return rtcSession;
}

RtcSession * RtcServer::FindSession(RtcRequest *request)
{
    return this->rtcMaster->FindSession(request->stream, request->session);
}

void RtcServer::DeleteSession(RtcSession *session)
{
    auto streamId = session->GetStreamId();

    PMS_INFO("SessionId[{}] StreamId[{}], RTC Session deleted, session ptr[{}].",
            session->GetSessionId(), streamId, (void *)session);

    Context *ctx = (Context *) session->GetContext();

    if (ctx) {
        session->SetContext(nullptr);

        if (ctx->nc) {
            ctx->nc->SetSession(nullptr);
            auto it = this->waittingPlayRequestMap.find(session->GetStreamId());
            if (it != this->waittingPlayRequestMap.end()) {
                for (auto requestIt = it->second.begin(); requestIt != it->second.end(); ++requestIt) {
                    auto request = *requestIt;
                    if (request->nc == ctx->nc) {
                        it->second.erase(requestIt);
                        break;
                    }
                }
            }
        }

        delete ctx;
    }

    this->rtcMaster->DeleteSession(streamId, session->GetSessionId());
}

}
