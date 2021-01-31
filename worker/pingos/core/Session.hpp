#pragma once
#include <map>
#include <string>
#include "RTC/RtpDictionaries.hpp"

namespace pingos
{
    class Session {
    public:
        Session(std::string streamId, std::string sessionId, bool publisher = false);
        virtual ~Session();

        void SetContext(std::string moduleName, void *context);
        void* GetContext(std::string moduleName);

        bool IsPublisher();

        std::string GetSessionId();
        std::string GetStreamId();

    private:
        std::map<std::string, void*> ctxMap;
        bool publisher { false };
        std::string sessionId { "" };
        std::string streamId { "" };
    };
} // namespace pingos
