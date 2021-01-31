#include "Module.hpp"

namespace pingos
{
    std::function< int(Session*, ConsoleRequest &r)> RequestConsoleFunctional;
    std::function< int(Session*)> PlayFunctional;
    std::function< int(Session*)> PublishFunctional;
    std::function< int(Session*)> CloseSessionFunctional;

    std::list<Module*> Module::modules;

    Module::Module(std::string name) : moduleName(name)
    {
    }

    void* Module::GetModuleCtx(Session *s)
    {
        return s->GetContext(this->moduleName);
    }

    void Module::SetModuleCtx(Session *s, void *ctx)
    {
        s->SetContext(this->moduleName, ctx);
    }

    std::string Module::GetName()
    {
        return this->moduleName;
    }

    void Module::ClassInit()
    {
        for (auto &module : modules) {
            module->ModuleInit();
        }
    }

    void Module::Register(Module *module)
    {
        modules.push_back(module);
    }
}
