#pragma once

#include "Module.hpp"
#include "Worker.hpp"
#include "ConsoleChannel.hpp"

namespace pingos
{
    class ConsoleModule : public Module , public ::Worker {
    public:
        ConsoleModule();
        virtual ~ConsoleModule();

    public:
        void InitModule() override;
    };
} // namespace pingos
