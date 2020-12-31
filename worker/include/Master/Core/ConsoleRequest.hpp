#pragma once
#include "Channel/Request.hpp"

namespace pingos
{
    class ConsoleRequest : public Channel::Request {
    public:
        ConsoleRequest(json& jsonRequest);
        virtual ~ConsoleRequest();
        virtual void Accept() override;
        virtual void Accept(json& data) override;
        virtual void Error(const char* reason = nullptr) override;
        virtual void TypeError(const char* reason = nullptr) override;

    public:
        int error { 0 };
        json acceptData;
        std::string reason { "" };
    };
} // namespace pingos
