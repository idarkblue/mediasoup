#pragma once

#include "core/Module.hpp"
#include "NetServer.hpp"


namespace pingos
{
    class RtspModule;
    class RtspModule : public Module, public NetServer::Listener {
    public:
        RtspModule();
        virtual ~RtspModule();

    public:
        static void ClassInit();
        static RtspModule* Get();

    // pingos::Module
    protected:
        void ModuleInit() override;

    public:
        void OnNetConnected(NetConnection *nc) override;
        void OnNetDataReceived(NetConnection *nc) override;
        void OnNetDisconnected(NetConnection *nc) override;

    private:
        static int PublishHandle(Session *s);
        static int PlayHandle(Session *s);
        static int CloseSessionHandle(Session *s);

    private:
        static RtspModule *instance;
    };
}
