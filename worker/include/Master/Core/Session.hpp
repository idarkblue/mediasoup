#pragma once
#include <map>
#include <string>

namespace pingos
{
    class Session {
    public:
        Session();
        virtual ~Session();
        void SetContext(std::string moduleName, void *context);
        void* GetContext(std::string moduleName);

    private:
        std::map<std::string, void*> contextMap;
    };
} // namespace pingos
