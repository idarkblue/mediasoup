#pragma once

#include <string>
#include <iostream>
#include <vector>
#include "PipeServer.hpp"
#include "PipeClient.hpp"
#include "UnixStreamSocket.hpp"

extern "C" {
    #include <uv.h>
}

namespace pingos {

class Worker : public PipeServer::Listener
{
public:
    class Listener {
    public:
        virtual void OnWorkerExited(Worker *w) = 0;
    };

public:
    struct Options {
        uv_loop_t               *loop;
        std::string              file;
        Listener                *listener;
        uint32_t                 slot;
    };

public:
    Worker(Options &opt);
    virtual ~Worker();

    void SetUnixStreamSocket(std::shared_ptr<UnixStreamSocket> channel);
    int SetListener(Listener *listener);
    int Spawn();

public:
    void OnWorkerExited(uv_process_t *req, int64_t status, int termSignal);

protected:
    int SetPipe();

// PipeServer listener
public:
    virtual void OnChannelAccept(PipeServer *ps, UnixStreamSocket *channel) override;
    virtual void OnChannelClosed(PipeServer *ps, UnixStreamSocket *channel) override;
    virtual void OnChannelRecv(PipeServer *ps, UnixStreamSocket *channel, std::string_view &payload) override;

protected:
    int ChannelSend(std::string data);

public:
    virtual void ReceiveChannelMessage(std::string_view &payload) = 0;
    virtual int ReceiveMasterMessage(std::string &payload) = 0;

private:
    uv_process_options_t m_options;
    uv_process_t         m_process;
    Options              m_opt;
    PipeClient          *m_pipeClient[4] { nullptr, nullptr };
    UnixStreamSocket    *m_channel[4] { nullptr };
    PipeServer          *m_pipeServer[2] { nullptr };
    std::string          m_pipeFile;
    std::string          m_pipePayloadFile;
    char               **m_args;
};

}
