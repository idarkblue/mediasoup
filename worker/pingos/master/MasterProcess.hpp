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

#include "Subprocess.hpp"

namespace pingos {

class MasterProcess : public Subprocess::Listener
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
    virtual Subprocess* NewSubprocess(uv_loop_t *loop) = 0;
    virtual void DeleteSubprocess(Subprocess *subprocess) = 0;

// Subprocess process listener
public:
    virtual void OnSubprocessExited(Subprocess *) override;

protected:
    uint32_t                           processNum { 0 };
    std::string                        execDir { "./" };
    std::string                        subprocessFile { "" };
    std::string                        subprocessName { "" };
    std::map<uint32_t, Subprocess*>    slotMap;
};

}
