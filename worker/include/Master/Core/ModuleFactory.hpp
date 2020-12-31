#pragma once

#include <list>
#include "Module.hpp"

namespace pingos
{
    class ModuleFactory {
    public:
        ModuleFactory() = default;
        virtual ~ModuleFactory() = default;

    public:
        static void Register(Module *module);
        static void LoadModules();

    private:
        static std::list<Module *> modules;
    };
} // namespace pingos
