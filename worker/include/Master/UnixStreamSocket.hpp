#pragma once

#include "Defines.hpp"
#include "handles/UnixStreamSocket.hpp"

namespace Master {

class WorkerProcess;

class UnixStreamSocket : public ::UnixStreamSocket
{
public:
    class Listener
    {
    public:
        virtual void OnChannelMessage(Master::UnixStreamSocket* channel, std::string_view &payload) = 0;
        virtual void OnChannelClosed(Master::UnixStreamSocket* channel) = 0;
    };

public:
    UnixStreamSocket(uv_pipe_t *handle, Listener* listener);
    virtual ~UnixStreamSocket();

public:
    WorkerProcess* GetWorkerProcess();

public:
    void SetListener(Listener* listener);
    void Send(json& jsonMessage);
    void SendBinary(const uint8_t* nsPayload, size_t nsPayloadLen);

    /* Pure virtual methods inherited from ::UnixStreamSocket. */
public:
    void UserOnUnixStreamRead() override;
    void UserOnUnixStreamSocketClosed() override;

private:
    void SendImpl(const void* nsPayload, size_t nsPayloadLen);

private:
    // Passed by argument.
    Listener *m_listener{ nullptr };
    WorkerProcess *m_wp { nullptr };
    // Others.
    size_t m_msgStart{ 0u }; // Where the latest message starts.
};

}
