#include "ConsoleModule.hpp"
#include "utils/JsonHelper.hpp"

#define MS_CLASS "pingos::ConsoleModule"

namespace pingos
{
    ConsoleModule *ConsoleModule::instance = nullptr;
    uint64_t ConsoleModule::requestId = 0;

    static std::function< int(Session*, ConsoleRequest &r)> NextRequestConsoleFunctional = nullptr;
    static std::function< int(Session*)> NextPublishFunctional = nullptr;
    static std::function< int(Session*)> NextPlayFunctional = nullptr;
    static std::function< int(Session*)> NextCloseSessionFunctional = nullptr;

    ConsoleModule::ConsoleModule() : Module("ConsoleModule"), ::Worker(nullptr, nullptr)
    {

    }

    ConsoleModule::~ConsoleModule()
    {

    }

    void ConsoleModule::ClassInit()
    {
        Module::Register(ConsoleModule::Get());
    }

    ConsoleModule* ConsoleModule::Get()
    {
        if (instance == nullptr) {
            instance = new ConsoleModule();
        }

        return instance;
    }

    void ConsoleModule::ModuleInit()
    {
        NextRequestConsoleFunctional = RequestConsoleFunctional;
        RequestConsoleFunctional = ConsoleModule::RequestConsoleHandle;

        NextPublishFunctional = PublishFunctional;
        PublishFunctional = ConsoleModule::PublishHandle;

        NextPlayFunctional = PlayFunctional;
        PlayFunctional = ConsoleModule::PlayHandle;

        NextCloseSessionFunctional = CloseSessionFunctional;
        CloseSessionFunctional = ConsoleModule::CloseSessionHandle;
    }

    void ConsoleModule::OnChannelEventActived(json &jsonMessage)
    {
        MS_DEBUG("Event : %s", jsonMessage.dump().c_str());
    }

    int ConsoleModule::RequestConsoleHandle(Session *s, ConsoleRequest &r)
    {
        // may throw
        ConsoleModule::Get()->OnChannelRequest(nullptr, &r);

        if (NextRequestConsoleFunctional) {
            return NextRequestConsoleFunctional(s, r);
        }

        return 0;
    }

    int ConsoleModule::PublishHandle(Session *s)
    {
        if (NextPublishFunctional) {
            return NextPublishFunctional(s);
        }

        return 0;
    }

    int ConsoleModule::PlayHandle(Session *s)
    {
        if (NextPlayFunctional) {
            return NextPlayFunctional(s);
        }

        return 0;
    }

    int ConsoleModule::CloseSessionHandle(Session *s)
    {
        if (NextCloseSessionFunctional) {
            return NextCloseSessionFunctional(s);
        }

        return 0;
    }
}
