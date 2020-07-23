#ifndef _MasterProcess_HPP_INCLUDE_
#define _MasterProcess_HPP_INCLUDE_

extern "C" {
    #include <inttypes.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "uv.h"
}

#include <map>
#include <string>
#include <iostream>
#include <vector>
#include "LiveStream.hpp"
#include "Worker.hpp"
#include "Filter.hpp"
#include "PipeServer.hpp"
#include "PipeClient.hpp"
#include "WorkerProcess.hpp"

namespace Master {

class MasterProcess : public WorkerProcess::Listener, public Filter
{

public:
    struct Options {
        std::string execDir { "./" };
        std::string workerName { "mediasoup-worker" };
        int nWorkers { 0 };
        bool daemon { false };
    };

public:
    static uv_loop_t  *Loop;

public:
    static void ClassInit(uv_loop_t *loop);
protected:
    static void OnWorkerExited(uv_process_t *req, int64_t exit_status, int term_signal);

public:
    MasterProcess();
    ~MasterProcess();

    void RegisterFilter(Filter *filter);

    int ConnectionFilter(RtcSession *s, ConnectionValue *val);
    int PlayFilter(RtcSession *s, PlayValue *val);
    int PublishFilter(RtcSession *s, PublishValue *val);
    int CloseStreamFilter(RtcSession *s, CloseStreamValue *val);

public:
    int ActiveWorkers(Options &opt);

protected:
    void SetPath(const char *execDir, const char *workerName);
    void SetCPU(int nWorkers);
    void CreateWorkers(bool daemon);

// Filter listener
public:
    virtual int OnConnection(RtcSession *s, ConnectionValue *val) override;
    virtual int OnPlay(RtcSession *s, PlayValue *val) override;
    virtual int OnPublish(RtcSession *s, PublishValue *val) override;
    virtual int OnCloseStream(RtcSession *s, CloseStreamValue *val) override;

// Worker process listener
public:
    virtual void OnWorkerProcessExited(WorkerProcess *) override;

private:
    std::map<std::string, LiveStream*>        m_streams;
    Filter                                   *m_filter { this };
    int                                       m_nWorkers { 0 };
    bool                                      m_daemon { false };
    std::string                               m_execDir { "./" };
    std::string                               m_workerPath { "" };
    std::string                               m_workerName { "" };
    std::vector<WorkerProcess *>              m_workers;
    std::map<UnixStreamSocket *, WorkerProcess*> m_sockWorkersMap;
};

}

#endif
