#pragma once

#include "Master.hpp"
#include "RtcWorker.hpp"

namespace pingos {

class RtcMaster : public Master {
public:
    RtcMaster();
    virtual ~RtcMaster();

    RtcWorker* FindWorker(uint32_t slot);

protected:
    virtual Worker* NewWorker(uv_loop_t *loop) override;
    virtual void DeleteWorker(Worker *worker) override;
};

}
