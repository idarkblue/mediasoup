#include "ConsoleRequest.hpp"

namespace pingos
{
    ConsoleRequest::ConsoleRequest(json& jsonRequest): Channel::Request(nullptr, jsonRequest)
    {

    }

    ConsoleRequest::~ConsoleRequest()
    {

    }

    void ConsoleRequest::Accept()
    {
        this->replied = true;
        this->error = 0;
    }

    void ConsoleRequest::Accept(json& data)
    {
        this->replied = true;
        this->error = 0;
        this->acceptData = data;
    }

    void ConsoleRequest::Error(const char* reason)
    {
        this->replied = true;
        this->error = -1;
        this->reason = reason;
    }

    void ConsoleRequest::TypeError(const char* reason)
    {
        this->replied = true;
        this->error = -1;
        this->reason = reason;
    }
} // namespace pingos
