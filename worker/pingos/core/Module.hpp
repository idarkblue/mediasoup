#pragma once

#include <string>
#include <list>
#include "Session.hpp"
#include "ConsoleRequest.hpp"

using json = nlohmann::json;

namespace pingos
{
    class Module;

    extern std::function< int(Session*, ConsoleRequest &)> RequestConsoleFunctional;
    extern std::function< int(Session*)> PlayFunctional;
    extern std::function< int(Session*)> PublishFunctional;
    extern std::function< int(Session*)> CloseSessionFunctional;

    class Module {
    public:
        Module(std::string name);
        Module() = default;
        virtual ~Module() = default;

        static void ClassInit();

    public:
        std::string GetName();
        void* GetModuleCtx(Session *s);
        void SetModuleCtx(Session *s, void *ctx);

    public:
        virtual void ModuleInit() = 0;

    protected:
        static void Register(Module *module);

        static std::list<Module*> modules;

    protected:
        std::string moduleName;
    };
}
