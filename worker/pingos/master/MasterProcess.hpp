#pragma once

// extern "C" {
//     #include <inttypes.h>
//     #include <stdio.h>
//     #include <stdlib.h>
//     #include <string.h>
// }

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <vector>
#include "uv.h"

#include "SubprocessAgent.hpp"

namespace pingos {

class MasterProcess : public SubprocessAgent::Listener
{
public:
    static uv_loop_t  *Loop;

public:
    static void ClassInit(uv_loop_t *loop);

public:
    MasterProcess();
    virtual ~MasterProcess();

public:
    int Start();
    uint32_t GetSubprocessCount();

protected:
    void SetPath();
    void SetCPU();
    void CreateSubprocess();

protected:
    int Send2Subprocess(uint32_t slot, std::string payload);

protected:
    virtual SubprocessAgent* NewSubprocess(uv_loop_t *loop) = 0;
    virtual void DeleteSubprocess(SubprocessAgent *subprocess) = 0;

// SubprocessAgent process listener
public:
    virtual void OnSubprocessExited(SubprocessAgent *) override;

protected:
    uint32_t                           processNum { 0 };
    std::string                        execDir { "./" };
    std::string                        subprocessFile { "" };
    std::string                        subprocessName { "" };
    std::map<uint32_t, SubprocessAgent*>    slotMap;
};

}
