#pragma once

#include "MainProcess.hpp"

namespace pingos
{
    class RtcMaster;

    class RtcMaster : public MainProcess {
    public:
        virtual SubProcess* NewProcess(uv_loop_t *loop) override;
        virtual void DeleteProcess(SubProcess *subprocess) override;
        virtual void OnProcessExited(SubProcess *subprocess) override;

    public:
        static RtcMaster *Instance();

    private:
        static RtcMaster *instance;
    };
} // namespace pingos
