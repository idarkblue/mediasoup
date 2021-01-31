#include "RtspModule.hpp"

namespace pingos
{
    static std::function< int(Session*)> NextPublishFunctional = nullptr;
    static std::function< int(Session*)> NextPlayFunctional = nullptr;
    static std::function< int(Session*)> NextCloseSessionFunctional = nullptr;

    RtspModule* RtspModule::instance = nullptr;

    RtspModule::RtspModule(): Module("RtspModule")
    {

    }

    RtspModule::~RtspModule()
    {

    }

    void RtspModule::ClassInit()
    {
        Module::Register(RtspModule::Get());
    }

    RtspModule* RtspModule::Get()
    {
        if (instance == nullptr) {
            instance = new RtspModule();
        }

        return instance;
    }

    void RtspModule::OnNetConnected(NetConnection *nc)
    {

    }

    void RtspModule::OnNetDataReceived(NetConnection *nc)
    {

    }

    void RtspModule::OnNetDisconnected(NetConnection *nc)
    {

    }

    void RtspModule::ModuleInit()
    {
        NextPublishFunctional = PublishFunctional;
        PublishFunctional = RtspModule::PublishHandle;

        NextPlayFunctional = PlayFunctional;
        PlayFunctional = RtspModule::PlayHandle;

        NextCloseSessionFunctional = CloseSessionFunctional;
        CloseSessionFunctional = RtspModule::CloseSessionHandle;
    }

    int RtspModule::PublishHandle(Session *s)
    {
        if (NextPublishFunctional) {
            return NextPublishFunctional(s);
        }
        return 0;
    }

    int RtspModule::PlayHandle(Session *s)
    {
        if (NextPlayFunctional) {
            return NextPlayFunctional(s);
        }
        return 0;
    }

    int RtspModule::CloseSessionHandle(Session *s)
    {
        if (NextCloseSessionFunctional) {
            return NextCloseSessionFunctional(s);
        }
        return 0;
    }
}
