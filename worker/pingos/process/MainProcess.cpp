#define PMS_CLASS "pingos::MainProcess"

#ifndef MAX_PATH
#define MAX_PATH 65535
#endif

#include "logger/Log.hpp"
#include "core/Configuration.hpp"
#include "MainProcess.hpp"

namespace pingos {

    uv_loop_t  *MainProcess::Loop = nullptr;

    void MainProcess::ClassInit(uv_loop_t *loop)
    {
        MainProcess::Loop = loop;
    }

    MainProcess::MainProcess()
    {

    }

    MainProcess::~MainProcess()
    {

    }

    uint32_t MainProcess::GetSubprocessCount()
    {
        return (uint32_t) slotMap.size();
    }

    int MainProcess::Start()
    {
        this->SetCPU();
        this->SetPath();
        this->CreateSubprocess();

        return 0;
    }

    void MainProcess::SetCPU()
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

    void MainProcess::SetPath()
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

    void MainProcess::CreateSubprocess()
    {
        int slot = 0;
        for (uint32_t i = 0; i < processNum; i++) {
            auto subprocess = this->NewProcess(MainProcess::Loop);

            if (!subprocess || subprocess->Start(slot, subprocessFile) != 0) {
                PMS_ERROR("Create subprocess failed.");
                continue;
            }

            slotMap[slot] = subprocess;

            slot++;
        }
    }

    int MainProcess::Broadcast2Worker(std::string payload)
    {
        for (auto &kv : this->slotMap) {
            this->Send2Worker(kv.first, payload);
        }

        return 0;
    }

    int MainProcess::Send2Worker(uint32_t slot, std::string payload)
    {
        if (slotMap.find(slot) == slotMap.end()) {
            PMS_ERROR("Send to subprocess[{}] failed, subprocess[{}] not found.", slot, slot);

            return -1;
        }

        auto subprocess = slotMap[slot];
        if (!subprocess) {
            PMS_ERROR("Send to subprocess[{}] failed, subprocess[{}] is nullptr.", slot, slot);
            return -1;
        }

        return 0;
    }

    void MainProcess::OnProcessExited(SubProcess * sp)
    {

    }

}
