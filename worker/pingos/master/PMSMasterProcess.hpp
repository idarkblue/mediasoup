#pragma once

#include "MasterProcess.hpp"

namespace pingos
{
    class PMSMasterProcess : public MasterProcess {
    public:
        PMSMasterProcess() = default;
        virtual ~PMSMasterProcess() = default;

    protected:
        SubprocessAgent* NewSubprocess(uv_loop_t *loop) override;
        void DeleteSubprocess(SubprocessAgent *subprocess) override;

    };
} // namespace pingos
