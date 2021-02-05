#pragma once

#include "SubprocessAgent.hpp"

namespace pingos
{
    class PMSSubprocess : public SubprocessAgent {
    public:
        PMSSubprocess() = default;
        PMSSubprocess(uv_loop_t *loop);
        virtual ~PMSSubprocess() = default;

    protected:
        void ReceiveChannelMessage(std::string_view &payload) override;
        int ReceiveMasterMessage(std::string &payload) override;
    };
} // namespace pingos
