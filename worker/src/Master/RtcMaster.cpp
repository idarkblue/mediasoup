#define PMS_CLASS "pingos::RtcMaster"

#include "Master/Defines.hpp"
#include "Master/RtcMaster.hpp"
#include "Master/Log.hpp"
#include "Master/RtcWorker.hpp"

namespace pingos {

RtcMaster::RtcMaster()
{
}

RtcMaster::~RtcMaster()
{

}

RtcWorker* RtcMaster::FindWorker(uint32_t slot)
{
    auto it = m_slotWorkerMap.find(slot);

    if (it != m_slotWorkerMap.end()) {
        return (RtcWorker*) it->second;
    }

    return nullptr;
}

Worker* RtcMaster::NewWorker(Worker::Options &opt)
{
    return new RtcWorker(opt);
}

void RtcMaster::DeleteWorker(Worker *worker)
{
    delete (RtcWorker*)worker;
}

}
