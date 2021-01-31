#pragma once

#include "core/Module.hpp"
#include "core/ConsoleRequest.hpp"
#include "core/ConsoleChannel.hpp"
#include "Worker.hpp"

namespace pingos
{
    class ConsoleModule;

    class ConsoleModule : public Module, public ::Worker, public ConsoleChannel::EventListener {

    public:
        ConsoleModule();
        virtual ~ConsoleModule();

    public:
        static void ClassInit();
        static ConsoleModule* Get();

    // pingos::Module
    protected:
        void ModuleInit() override;

    // ConsoleChannel::EventListener
    public:
        void OnChannelEventActived(json &jsonMessage) override;

    private:
        static int RequestConsoleHandle(Session *s, ConsoleRequest &r);
        static int PublishHandle(Session *s);
        static int PlayHandle(Session *s);
        static int CloseSessionHandle(Session *s);

    private:
        static ConsoleModule *instance;
        static uint64_t requestId;
    };
} // namespace pingos
