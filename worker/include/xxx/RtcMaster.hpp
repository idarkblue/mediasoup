#pragma once

#include "Master.hpp"
#include "RtcWorker.hpp"
#include "RtcSession.hpp"

namespace pingos {

class RtcMaster : public Master {
public:
    RtcMaster();
    virtual ~RtcMaster();

    RtcWorker* FindWorker(uint32_t slot);
    RtcWorker* FindWorker(std::string streamId);
    RtcSession* FindPublisher(std::string streamId);
    RtcSession* CreateSession(std::string streamId, RtcSession::Role role);
    RtcSession* FindSession(std::string streamId, std::string sessionId);
    void DeleteSession(std::string streamId, std::string esssionId);

protected:
    virtual Worker* NewProcess(uv_loop_t *loop) override;
    virtual void DeleteProcess(Worker *worker) override;

private:
    std::string SpellSessionId();
};

}
