#define PMS_CLASS "pingos::RtcWorker"
#define MS_CLASS "pingos::RtcWorker"

#include "Master/Log.hpp"
#include "Master/RtcWorker.hpp"
#include "MediaSoupErrors.hpp"

namespace pingos {

std::unordered_map<std::string, RtcStream*> RtcWorker::m_streamsMap;

RtcWorker::RtcWorker(Worker::Options &opt): Worker(opt)
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

    m_requestWaittingMap[request.GetId()] = info;

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

    auto it = m_requestWaittingMap.find(id);
    if (it == m_requestWaittingMap.end()) {
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

RtcSession *RtcWorker::FindRtcSession(std::string streamId, std::string sessionId)
{
    if (m_streamsMap.count(streamId) == 0 || !m_streamsMap[streamId]) {
        return nullptr;
    }

    auto *rtcSession = m_streamsMap[streamId]->GetPublisher();
    if (rtcSession == nullptr) {
        rtcSession = m_streamsMap[streamId]->GetPlayer(sessionId);
    }

    return rtcSession;
}

RtcSession *RtcWorker::CreateSession(std::string streamId, std::string sessionId, RtcSession::Role role)
{
    auto *rtcSession = this->FindRtcSession(streamId, sessionId);

    if (rtcSession) {
        PMS_ERROR("SessionId[{}] StreamId[{}], Session already exists",
            rtcSession->GetSessionId(), streamId);

        MS_THROW_ERROR("Session already exists");

        return nullptr;
    }

    rtcSession = new RtcSession(role, sessionId, streamId);
    if (!rtcSession) {
        PMS_ERROR("SessionId[{}] StreamId[{}], RTC Session creation failed.",
            sessionId, streamId);

        MS_THROW_ERROR("RTC Session creation failed");

        return nullptr;
    }

    rtcSession->SetWorker(this);

    if (!m_streamsMap.count(streamId) || !m_streamsMap[streamId]) {
        m_streamsMap[streamId] = new RtcStream(streamId);
    }

    auto *stream = m_streamsMap[streamId];
    if (stream->Join(rtcSession) != 0) {
        PMS_ERROR("SessionId[{}] StreamId[{}], RTC Session join failed.",
            sessionId, streamId);

        MS_THROW_ERROR("RTC Session join failed");

        return nullptr;
    }

    PMS_INFO("SessionId[{}] StreamId[{}], RTC Session creation success, session ptr[{}].",
            sessionId, streamId, (void *)rtcSession);

    return rtcSession;
}

void RtcWorker::DeleteSession(std::string streamId, std::string sessionId)
{
    if (m_streamsMap.count(streamId) == 0 || !m_streamsMap[streamId]) {
        PMS_ERROR("SessionId[{}] Stream[{}] delete session failed, stream not found", streamId, sessionId);
        return;
    }

    auto *rtcSession = m_streamsMap[streamId]->DeleteSession(sessionId);

    if (rtcSession) {
        delete rtcSession;
    }

    PMS_INFO("SessionId[{}] Stream[{}] delete session success, ptr[{}]",
        streamId, sessionId, (void*)rtcSession);

    return;
}

}
