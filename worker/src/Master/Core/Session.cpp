#include "Session.hpp"

namespace pingos
{
    Session::Session()
    {

    }

    Session::~Session()
    {

    }

    void Session::SetContext(std::string moduleName, void *context)
    {
        this->contextMap[moduleName] = context;
    }

    void* Session::GetContext(std::string moduleName)
    {
        auto it = this->contextMap.find(moduleName);
        if (it != this->contextMap.end()) {
            return nullptr;
        }

        return it->second;
    }

} // namespace pingos
