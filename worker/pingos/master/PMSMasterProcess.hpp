#pragma once

#include "MasterProcess.hpp"

namespace pingos
{
    class PMSMasterProcess : public MasterProcess {
    public:
        PMSMasterProcess() = default;
        virtual ~PMSMasterProcess() = default;

    protected:
        Subprocess* NewSubprocess(uv_loop_t *loop) override;
        void DeleteSubprocess(Subprocess *subprocess) override;

    };
} // namespace pingos
