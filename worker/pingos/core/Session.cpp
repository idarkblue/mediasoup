#include "Session.hpp"
#include "ConsoleFilter.hpp"

namespace pingos
{
    Session::Session(std::string streamId, std::string sessionId, bool publisher) :
        streamId(streamId), sessionId(sessionId), publisher(publisher)
    {

    }

    Session::~Session()
    {

    }

    void Session::SetContext(std::string moduleName, void *context)
    {
        this->ctxMap[moduleName] = context;
    }

    void* Session::GetContext(std::string moduleName)
    {
        auto it = this->ctxMap.find(moduleName);
        if (it == this->ctxMap.end()) {
            return nullptr;
        }

        return it->second;
    }

    bool Session::IsPublisher()
    {
        return this->publisher;
    }

    std::string Session::GetSessionId()
    {
        return this->sessionId;
    }

    std::string Session::GetStreamId()
    {
        return this->streamId;
    }

} // namespace pingos
