#pragma once

#include <json.hpp>
#include "Session.hpp"
#include "ConsoleRequest.hpp"
#include "Worker.hpp"

using json = nlohmann::json;

namespace pingos
{
    class ConsoleWorker : public Worker
    {
    public:
        ConsoleWorker(::Channel::Channel* channel, PayloadChannel::Channel* payloadChannel);
        virtual ~ConsoleWorker();

    public:
        void ConsoleRequestHandle(ConsoleRequest &request);
    };
} // namespace pingos
