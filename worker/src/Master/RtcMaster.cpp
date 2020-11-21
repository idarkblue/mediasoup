#define PMS_CLASS "pingos::RtcMaster"
#define MS_CLASS "pingos::RtcMaster"

#include "Master/Defines.hpp"
#include "Master/RtcMaster.hpp"
#include "Master/Log.hpp"
#include "Master/RtcWorker.hpp"
#include "MediaSoupErrors.hpp"

namespace pingos {

RtcMaster::RtcMaster()
{
}

RtcMaster::~RtcMaster()
{

}

RtcWorker* RtcMaster::FindWorker(std::string streamId)
{
    size_t hashVal = std::hash<std::string>()(streamId);
    size_t slot = hashVal % this->GetWorkerCount();

    return this->FindWorker(slot);
}

RtcSession* RtcMaster::CreateSession(std::string streamId, RtcSession::Role role)
{
    auto worker = this->FindWorker(streamId);
    if (!worker) {
        PMS_ERROR("StreamId[{}], Worker not found", streamId);

        MS_THROW_ERROR("Worker not found");

        return nullptr;
    }

    auto sessionId = this->SpellSessionId();
    RtcSession *rtcSession = worker->CreateSession(streamId, sessionId, role);
    if (!rtcSession) {
        PMS_ERROR("SessionId[{}] StreamId[{}] create session failed.", sessionId, streamId);

        return nullptr;
    }

    PMS_INFO("SessionId[{}] StreamId[{}], RTC Session creation success, session ptr[{}].",
            sessionId, streamId, (void *)rtcSession);

    return rtcSession;
}

RtcWorker* RtcMaster::FindWorker(uint32_t slot)
{
    auto it = m_slotWorkerMap.find(slot);

    if (it != m_slotWorkerMap.end()) {
        return (RtcWorker*) it->second;
    }

    return nullptr;
}

Worker* RtcMaster::NewWorker(uv_loop_t *loop)
{
    return new RtcWorker(loop);
}

void RtcMaster::DeleteWorker(Worker *worker)
{
    delete (RtcWorker*)worker;
}

std::string RtcMaster::SpellSessionId()
{
    static uint64_t id = 0;
    std::string sessionId = std::string("sid") + std::to_string(id);

    id++;

    return sessionId;
}

RtcSession* RtcMaster::FindPublisher(std::string streamId)
{
    auto worker = this->FindWorker(streamId);
    if (worker == nullptr) {
        return nullptr;
    }

    return worker->FindPublisher(streamId);
}

RtcSession* RtcMaster::FindSession(std::string streamId, std::string sessionId)
{
    auto worker = this->FindWorker(streamId);
    if (!worker) {
        PMS_ERROR("SessionId[{}] StreamId[{}] Failed to find worker", sessionId, streamId);
        return nullptr;
    }

    return worker->FindSession(streamId, sessionId);
}

void RtcMaster::DeleteSession(std::string streamId, std::string sessionId)
{
    auto worker = this->FindWorker(streamId);
    if (!worker) {
        PMS_ERROR("SessionId[{}] StreamId[{}], Worker not found", sessionId, streamId);
        return;
    }

    PMS_INFO("SessionId[{}] StreamId[{}], RTC Session deleted.", sessionId, streamId);

    worker->DeleteSession(streamId, sessionId);
}

}
