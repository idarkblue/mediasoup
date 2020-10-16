#define PMS_CLASS "pingos::Master"

#include "Master/Defines.hpp"
#include "Master/Log.hpp"
#include "Master/Master.hpp"

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
    return m_workers;
}

int Master::StartWorkers(Options &opt)
{
    this->SetCPU(opt.nWorkers);
    this->SetPath(opt.execDir.c_str(), opt.workerName.c_str());
    this->CreateWorkers(opt.daemon);

    return 0;
}

void Master::SetCPU(int nWorkers)
{
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

void Master::SetPath(const char *execDir, const char *name)
{
    m_execDir = execDir;
    if (execDir[strlen(execDir) - 1] != '/') {
        m_execDir += "/";
    }

    if (uv_chdir(m_execDir.c_str()) != 0) {
        PMS_ERROR("Changes the current working directory[{}] failed.", m_execDir.c_str());
        return;
    }

    size_t pathSize = MAX_PATH + 1;
    char MasterProcessPath[MAX_PATH + 1];

    uv_exepath(MasterProcessPath, &pathSize);

    std::string path(MasterProcessPath, pathSize);
    std::string exeDir = "./";

    auto pos = path.find_last_of('/');
    if (pos != std::string::npos) {
        exeDir = path.substr(0, pos + 1);
    }

    m_workerName = name;
    m_workerPath = exeDir + m_workerName;
}

void Master::CreateWorkers(bool daemon)
{
    m_daemon = daemon;

    for (uint32_t i = 0; i < m_workers; i++) {

        Worker::Options opt = {
            .loop     = Master::Loop,
            .file     = m_workerPath,
            .listener = this,
            .slot     = i
        };

        auto worker = this->NewWorker(opt);
        worker->Spawn();
        m_slotWorkerMap[i] = worker;
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
