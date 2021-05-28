#pragma once

#include <string>
#include <json.hpp>
#include "MediaSoupErrors.hpp"
#include "handles/UnixStreamSocket.hpp"

using json = nlohmann::json;

namespace pingos {

class UnixStreamSocket : public ::UnixStreamSocket
{
public:
    class Listener
    {
    public:
        virtual void OnChannelMessage(pingos::UnixStreamSocket* channel, std::string_view &payload) = 0;
        virtual void OnChannelClosed(pingos::UnixStreamSocket* channel) = 0;
    };

public:
    UnixStreamSocket(uv_pipe_t *handle, Listener* listener, ::UnixStreamSocket::Role);
    virtual ~UnixStreamSocket();

public:
    void SetListener(Listener* listener);
    void Send(json& jsonMessage);
    void SendBinary(const uint8_t* nsPayload, size_t nsPayloadLen);
    void SendString(std::string &message);

public:
    ::UnixStreamSocket::Role GetRole();

    /* Pure virtual methods inherited from ::UnixStreamSocket. */
public:
    void UserOnUnixStreamRead() override;
    void UserOnUnixStreamSocketClosed() override;

private:
    void SendImpl(const void* nsPayload, size_t nsPayloadLen);

private:
    // Passed by argument.
    Listener *listener{ nullptr };
    // Others.
    size_t msgStart{ 0u }; // Where the latest message starts.
};

}
