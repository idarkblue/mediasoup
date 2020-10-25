#define PMS_CLASS "pingos::Master"

#include "Master/Defines.hpp"
#include "Master/Log.hpp"
#include "Master/Master.hpp"
#include "Master/Configuration.hpp"

namespace pingos {

uv_loop_t  *Master::Loop = nullptr;

void Master::ClassInit(uv_loop_t *loop)
{
    Master::Loop = loop;
}

Master::Master()
{

}

Master::~Master()
{

}

uint32_t Master::GetWorkerCount()
{
    return (uint32_t) m_slotWorkerMap.size();
}

int Master::Start()
{
    this->SetCPU();
    this->SetPath();
    this->CreateWorkers();

    return 0;
}

void Master::SetCPU()
{
    int nWorkers = pingos::Configuration::master.numOfWorkerProcess;
    int            cpuCount;
    uv_cpu_info_t *info;

    if (nWorkers <=0 ) {
        uv_cpu_info(&info, &cpuCount);
        uv_free_cpu_info(info, cpuCount);
        m_workers = cpuCount;
    } else {
        m_workers = nWorkers;
    }
}

void Master::SetPath()
{
    m_execDir = pingos::Configuration::master.execPath;
    m_workerName = pingos::Configuration::master.workerName;

    if (uv_chdir(m_execDir.c_str()) != 0) {
        PMS_ERROR("Changes the current working directory[{}] failed.", m_execDir.c_str());
        return;
    }

    size_t pathSize = MAX_PATH + 1;
    char MasterProcessPath[MAX_PATH + 1];

    uv_exepath(MasterProcessPath, &pathSize);

    std::string path(MasterProcessPath, pathSize);
    std::string execPath = "./";

    auto pos = path.find_last_of('/');
    if (pos != std::string::npos) {
        execPath = path.substr(0, pos + 1);
    }

    m_workerFile = execPath + m_workerName;
}

void Master::CreateWorkers()
{
    int slot = 0;
    for (uint32_t i = 0; i < m_workers; i++) {
        auto worker = this->NewWorker(Master::Loop);

        if (!worker || worker->Start(slot, m_workerFile) != 0) {
            PMS_ERROR("Create worker failed.");
            continue;
        }

        m_slotWorkerMap[slot] = worker;

        slot++;
    }
}

int Master::Send2Worker(uint32_t slot, std::string payload)
{
    if (m_slotWorkerMap.find(slot) == m_slotWorkerMap.end()) {
        PMS_ERROR("Send to worker[{}] failed, worker[{}] not found.", slot, slot);

        return -1;
    }

    auto worker = m_slotWorkerMap[slot];
    if (!worker) {
        PMS_ERROR("Send to worker[{}] failed, worker[{}] is nullptr.", slot, slot);
        return -1;
    }

    worker->ReceiveMasterMessage(payload);

    return 0;
}

void Master::OnWorkerExited(Worker * wp)
{

}

}
