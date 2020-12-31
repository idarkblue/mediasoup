#pragma once

#include "json.hpp"
#include "Session.hpp"
#include "ConsoleRequest.hpp"

using json = nlohmann::json;

namespace pingos
{
    class Module {
    public:
        Module() = default;
        virtual ~Module() = default;

    public:
        virtual void InitModule() = 0;

        virtual int OnConsoleRequest(Session *s, ConsoleRequest &request) = 0;
        virtual int OnInitSession(Session *s) = 0;
        virtual int OnPlay(Session *s, json &jsonObject) = 0;
        virtual int OnPublish(Session *s, json &jsonObject) = 0;
        virtual int OnConnection(Session *s, json &jsonObject) = 0;
        virtual int OnCloseSession(Session *s) = 0;
    };
} // namespace pingos
