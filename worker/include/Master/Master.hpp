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
#include <vector>
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
    static uv_loop_t  *Loop;

public:
    static void ClassInit(uv_loop_t *loop);

public:
    Master();
    ~Master();

public:
    int Start();
    uint32_t GetWorkerCount();

protected:
    void SetPath();
    void SetCPU();
    void CreateWorkers();

protected:
    int Send2Worker(uint32_t slot, std::string payload);

protected:
    virtual Worker* NewWorker(uv_loop_t *loop) = 0;
    virtual void DeleteWorker(Worker *worker) = 0;

// Worker process listener
public:
    virtual void OnWorkerExited(Worker *) override;

protected:
    uint32_t                                  m_workers { 0 };
    std::string                               m_execDir { "./" };
    std::string                               m_workerFile { "" };
    std::string                               m_workerName { "" };
    std::map<uint32_t, Worker*>               m_slotWorkerMap;
};

}

#endif
