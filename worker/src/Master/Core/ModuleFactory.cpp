#include "ModuleFactory.hpp"

namespace pingos
{
    std::list<Module *> ModuleFactory::modules;
    void ModuleFactory::Register(Module *module)
    {
        if (module) {
            modules.push_back(module);
        }
    }

    void ModuleFactory::LoadModules()
    {
        for (auto &module : modules) {
            module->InitModule();
        }
    }
} // namespace pingos
