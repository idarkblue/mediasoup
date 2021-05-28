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

#include "SubProcess.hpp"

namespace pingos {

    class MainProcess : public SubProcess::Listener
    {
    public:
        static uv_loop_t  *Loop;

    public:
        static void ClassInit(uv_loop_t *loop);

    public:
        MainProcess();
        virtual ~MainProcess();

    public:
        int Start();
        uint32_t GetSubprocessCount();

    protected:
        void SetPath();
        void SetCPU();
        void CreateSubprocess();

    public:
        int Broadcast2Worker(std::string payload);
        int Send2Worker(uint32_t slot, std::string payload);

    protected:
        virtual SubProcess* NewProcess(uv_loop_t *loop) = 0;
        virtual void DeleteProcess(SubProcess *subprocess) = 0;

    // SubProcess process listener
    public:
        virtual void OnProcessExited(SubProcess *) override;

    protected:
        uint32_t                           processNum { 0 };
        std::string                        execDir { "./" };
        std::string                        subprocessFile { "" };
        std::string                        subprocessName { "" };
        std::map<uint32_t, SubProcess*>    slotMap;
    };

}
