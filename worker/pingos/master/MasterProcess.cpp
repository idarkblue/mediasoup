#define PMS_CLASS "pingos::MasterProcess"

#ifndef MAX_PATH
#define MAX_PATH 65535
#endif

#include "logger/Log.hpp"
#include "core/Configuration.hpp"
#include "MasterProcess.hpp"

namespace pingos {

uv_loop_t  *MasterProcess::Loop = nullptr;

void MasterProcess::ClassInit(uv_loop_t *loop)
{
    MasterProcess::Loop = loop;
}

MasterProcess::MasterProcess()
{

}

MasterProcess::~MasterProcess()
{

}

uint32_t MasterProcess::GetSubprocessCount()
{
    return (uint32_t) slotMap.size();
}

int MasterProcess::Start()
{
    this->SetCPU();
    this->SetPath();
    this->CreateSubprocess();

    return 0;
}

void MasterProcess::SetCPU()
{
    int nWorkers = pingos::Configuration::master.numOfWorkerProcess;
    int            cpuCount;
    uv_cpu_info_t *info;

    if (nWorkers <=0 ) {
        uv_cpu_info(&info, &cpuCount);
        uv_free_cpu_info(info, cpuCount);
        processNum = cpuCount;
    } else {
        processNum = nWorkers;
    }
}

void MasterProcess::SetPath()
{
    this->execDir = pingos::Configuration::master.execPath;
    this->subprocessName = pingos::Configuration::master.workerName;

    if (uv_chdir(this->execDir.c_str()) != 0) {
        PMS_ERROR("Changes the current working directory[{}] failed.", this->execDir.c_str());
        return;
    }

    static size_t pathSize = MAX_PATH + 1;
    static char MasterProcessPath[MAX_PATH + 1];

    uv_exepath(MasterProcessPath, &pathSize);

    std::string path(MasterProcessPath, pathSize);
    // std::string execPath = "./";

    // auto pos = path.find_last_of('/');
    // if (pos != std::string::npos) {
    //     execPath = path.substr(0, pos + 1);
    // }

    subprocessFile = path;
}

void MasterProcess::CreateSubprocess()
{
    int slot = 0;
    for (uint32_t i = 0; i < processNum; i++) {
        auto worker = this->NewSubprocess(MasterProcess::Loop);

        if (!worker || worker->Start(slot, subprocessFile) != 0) {
            PMS_ERROR("Create worker failed.");
            continue;
        }

        slotMap[slot] = worker;

        slot++;
    }
}

int MasterProcess::Send2Subprocess(uint32_t slot, std::string payload)
{
    if (slotMap.find(slot) == slotMap.end()) {
        PMS_ERROR("Send to worker[{}] failed, worker[{}] not found.", slot, slot);

        return -1;
    }

    auto worker = slotMap[slot];
    if (!worker) {
        PMS_ERROR("Send to worker[{}] failed, worker[{}] is nullptr.", slot, slot);
        return -1;
    }

    worker->ReceiveMasterMessage(payload);

    return 0;
}

void MasterProcess::OnSubprocessExited(SubprocessAgent * sp)
{

}

}
