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
#include "Worker.hpp"
#include "PipeServer.hpp"
#include "PipeClient.hpp"
#include "Worker.hpp"

namespace pingos {

class Master : public Worker::Listener
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

public:
    Master();
    ~Master();

public:
    int StartWorkers(Options &opt);
    uint32_t GetWorkerCount();

protected:
    void SetPath(const char *execDir, const char *workerName);
    void SetCPU(int nWorkers);
    void CreateWorkers(bool daemon);

protected:
    int Send2Worker(uint32_t slot, std::string payload);

protected:
    virtual Worker* NewWorker(Worker::Options &opt) = 0;
    virtual void DeleteWorker(Worker *worker) = 0;

// Worker process listener
public:
    virtual void OnWorkerExited(Worker *) override;

protected:
    uint32_t                                  m_workers { 0 };
    bool                                      m_daemon { false };
    std::string                               m_execDir { "./" };
    std::string                               m_workerPath { "" };
    std::string                               m_workerName { "" };
    std::map<uint32_t, Worker*>               m_slotWorkerMap;
};

}

#endif
