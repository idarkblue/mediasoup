#define PMS_CLASS "MasterProcess"

#include "Master/Defines.hpp"
#include "Master/Log.hpp"
#include "Master/MasterProcess.hpp"

namespace Master {

uv_loop_t  *MasterProcess::Loop = nullptr;

void MasterProcess::ClassInit(uv_loop_t *loop)
{
    MasterProcess::Loop = loop;
}

int MasterProcess::ActiveWorkers(Options &opt)
{
    this->SetCPU(opt.nWorkers);
    this->SetPath(opt.execDir.c_str(), opt.workerName.c_str());
    this->CreateWorkers(opt.daemon);

    return 0;
}

void MasterProcess::SetCPU(int nWorkers)
{
    int            cpuCount;
    uv_cpu_info_t *info;

    if (nWorkers <=0 ) {
        uv_cpu_info(&info, &cpuCount);
        uv_free_cpu_info(info, cpuCount);
        m_nWorkers = cpuCount;
    } else {
        m_nWorkers = nWorkers;
    }
}

void MasterProcess::SetPath(const char *execDir, const char *name)
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

void MasterProcess::CreateWorkers(bool daemon)
{
    m_daemon = daemon;

    for (int i = 0; i < m_nWorkers; i++) {

        WorkerProcess::Options opt = {
            .loop     = MasterProcess::Loop,
            .file     = m_workerPath,
            .listener = this,
            .slot     = i
        };

        auto worker = new WorkerProcess(opt);
        worker->Run();
        m_workers.push_back(worker);
    }
}

void MasterProcess::OnWorkerExited(uv_process_t *req, int64_t status, int termSignal)
{
    PMS_ERROR("Worker process[{}] exited\r\n", req->pid);
}

MasterProcess::MasterProcess()
{

}

MasterProcess::~MasterProcess()
{

}

void MasterProcess::RegisterFilter(Filter *filter)
{
    filter->SetNext(m_filter);

    m_filter = filter;
}

int MasterProcess::PlayFilter(RtcSession *s, PlayValue *val)
{
    m_workers[0]->CreateRouter("router-0001");
    return m_filter->OnPlay(s, val);
}

int MasterProcess::PublishFilter(RtcSession *s, PublishValue *val)
{
    return m_filter->OnPublish(s, val);
}

int MasterProcess::CloseStreamFilter(RtcSession *s, CloseStreamValue *val)
{
    return m_filter->OnCloseStream(s, val);
}

int MasterProcess::OnPlay(RtcSession *s, PlayValue *val)
{
    return 0;
}

int MasterProcess::OnPublish(RtcSession *s, PublishValue *val)
{
    return 0;
}

int MasterProcess::OnCloseStream(RtcSession *s, CloseStreamValue *val)
{
    return 0;
}

void MasterProcess::OnWorkerProcessExited(WorkerProcess * wp)
{

}

}
