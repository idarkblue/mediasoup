#pragma once

#include <string>
#include <iostream>
#include <vector>
#include "UnixStreamSocket.hpp"
#include "Pipe.hpp"

extern "C" {
    #include <uv.h>
}

namespace pingos {

class Worker : public UnixStreamSocket::Listener
{
public:
    class Listener {
    public:
        virtual void OnWorkerExited(Worker *w) = 0;
    };

public:
    Worker(uv_loop_t *loop);
    virtual ~Worker();

    int Start(int slot, std::string file);
    int SetListener(Listener *listener);

public:
    void OnWorkerExited(uv_process_t *req, int64_t status, int termSignal);

// UnixStreamSocket listener
public:
    virtual void OnChannelMessage(pingos::UnixStreamSocket* channel, std::string_view &payload) override;
    virtual void OnChannelClosed(pingos::UnixStreamSocket* channel) override;

protected:
    int InitChannels();
    int Spawn();
    int ChannelSend(std::string data);

public:
    virtual void ReceiveChannelMessage(std::string_view &payload) = 0;
    virtual int ReceiveMasterMessage(std::string &payload) = 0;

private:
    uv_process_options_t m_options;
    uv_process_t         m_process;
    Listener            *m_listener {nullptr};
    UnixStreamSocket    *m_channelIn { nullptr };
    UnixStreamSocket    *m_channelOut { nullptr };
    UnixStreamSocket    *m_payloadChannelIn { nullptr };
    UnixStreamSocket    *m_payloadChannelOut { nullptr };

    Pipe                 m_channelPipe;
    Pipe                 m_payloadChannelPipe;

    char               **m_args { nullptr };
    int                  m_slot { -1 };
    uv_loop_t           *m_loop { nullptr };
    std::string          m_file { "" };

    uv_stdio_container_t childStdio[7];
    char *env[2];
};

}
