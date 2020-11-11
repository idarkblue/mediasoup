#define PMS_CLASS "pingos::RtcWorker"
#define MS_CLASS "pingos::RtcWorker"

#include "Master/Log.hpp"
#include "Master/RtcWorker.hpp"
#include "MediaSoupErrors.hpp"

namespace pingos {

std::unordered_map<std::string, RtcStream*> RtcWorker::streamsMap;

RtcWorker::RtcWorker(uv_loop_t *loop): Worker(loop)
{
}

RtcWorker::~RtcWorker()
{
}

int RtcWorker::ReceiveMasterMessage(std::string &payload)
{
    return 0;
}

int RtcWorker::SendRequest(RtcSession *rtcSession, RtcSession::Request &request)
{
    json jsonObject = json::object();

    request.FillJson(jsonObject);

    std::string payload = jsonObject.dump();

    if (this->ChannelSend(payload)) {
        PMS_ERROR("SessionId[{}] streamId[{}] Send request[{}] failed, content: {}",
            rtcSession->GetSessionId(), rtcSession->GetStreamId(), request.GetId(), payload);
        return -1;
    }

    RequestWaittingInfo info;
    info.sessionId = rtcSession->GetSessionId();
    info.streamId = rtcSession->GetStreamId();

    this->requestWaittingMap[request.GetId()] = info;

    return 0;
}

void RtcWorker::ReceiveChannelMessage(std::string_view &payload)
{
    json jsonObject = json::parse(payload);
    if (jsonObject.find("event") != jsonObject.end()) {
        // receive event
        this->ReceiveChannelEvent(jsonObject);
    } else {
        // receive ack
        this->ReceiveChannelAck(jsonObject);
    }
}

void RtcWorker::ReceiveChannelAck(json &jsonObject)
{
    if (jsonObject.find("id") == jsonObject.end()) {
        PMS_ERROR("Invalid channel ack, miss id, data {}", jsonObject.dump());
        return;
    }

    auto id = jsonObject["id"].get<uint64_t>();

    auto it = this->requestWaittingMap.find(id);
    if (it == this->requestWaittingMap.end()) {
        PMS_ERROR("Invalid channel ack, miss request, data {}", jsonObject.dump());
        return;
    }

    auto &info = it->second;

    auto rtcSession = this->FindRtcSession(info.streamId, info.sessionId);
    if (rtcSession == nullptr) {
        PMS_ERROR("SessionId[{}] StreamId[{}] Invalid channel ack, miss rtc session, data {}",
            info.sessionId, info.streamId, jsonObject.dump());
        return;
    }

    rtcSession->ReceiveChannelAck(jsonObject);
}

void RtcWorker::ReceiveChannelEvent(json &jsonObject)
{

}

RtcSession *RtcWorker::FindPublisher(std::string streamId)
{
    if (RtcWorker::streamsMap.count(streamId) == 0 || !RtcWorker::streamsMap[streamId]) {
        return nullptr;
    }

    return RtcWorker::streamsMap[streamId]->GetPublisher();
}

RtcSession *RtcWorker::FindRtcSession(std::string streamId, std::string sessionId)
{
    if (RtcWorker::streamsMap.count(streamId) == 0 || !RtcWorker::streamsMap[streamId]) {
        return nullptr;
    }

    auto *rtcSession = RtcWorker::streamsMap[streamId]->GetPublisher();
    if (rtcSession && rtcSession->GetSessionId() == sessionId) {
        return rtcSession;
    }

    return RtcWorker::streamsMap[streamId]->GetPlayer(sessionId);
}

RtcSession *RtcWorker::CreateSession(std::string streamId, std::string sessionId, RtcSession::Role role)
{
    auto *rtcSession = this->FindRtcSession(streamId, sessionId);

    if (rtcSession) {
        PMS_ERROR("SessionId[{}] StreamId[{}], Session already exists",
            rtcSession->GetSessionId(), streamId);

        return nullptr;
    }

    rtcSession = new RtcSession(role, sessionId, streamId);
    if (!rtcSession) {
        PMS_ERROR("SessionId[{}] StreamId[{}], RTC Session creation failed.",
            sessionId, streamId);

        return nullptr;
    }

    rtcSession->SetWorker(this);

    if (!RtcWorker::streamsMap.count(streamId) || !RtcWorker::streamsMap[streamId]) {
        RtcWorker::streamsMap[streamId] = new RtcStream(streamId);
    }

    auto *stream = RtcWorker::streamsMap[streamId];
    if (stream->Join(rtcSession) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}], RTC Session join failed.",
            sessionId, streamId);

        return nullptr;
    }

    PMS_INFO("SessionId[{}] StreamId[{}], RTC Session creation success, session ptr[{}].",
            sessionId, streamId, (void *)rtcSession);

    return rtcSession;
}

void RtcWorker::DeleteSession(std::string streamId, std::string sessionId)
{
    if (RtcWorker::streamsMap.count(streamId) == 0 || !RtcWorker::streamsMap[streamId]) {
        PMS_ERROR("SessionId[{}] Stream[{}] delete session failed, stream not found", streamId, sessionId);
        return;
    }

    auto *rtcSession = RtcWorker::streamsMap[streamId]->RemoveSession(sessionId);

    PMS_INFO("SessionId[{}] Stream[{}] delete session success, ptr[{}]",
        sessionId, streamId, (void*)rtcSession);

    if (rtcSession) {
        delete rtcSession;
    }

    return;
}

}
