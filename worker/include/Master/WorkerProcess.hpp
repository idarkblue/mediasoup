#pragma once

#include <string>
#include <iostream>
#include "PipeServer.hpp"
#include "PipeClient.hpp"
#include "UnixStreamSocket.hpp"

extern "C" {
    #include <uv.h>
}

namespace Master {

class WorkerProcess : public PipeServer::Listener
{
public:
    class Listener {
    public:
        virtual void OnWorkerProcessExited(WorkerProcess *wp) = 0;
    };

public:
    struct Options {
        uv_loop_t           *loop;
        std::string          file;
        Listener            *listener;
        int                  slot;
    };

public:
    static void ClassInit(const char **args, const char *pipePath);
private:
    static char **processArgs;
    static std::string pipePath;

public:
    WorkerProcess(Options &opt);
    virtual ~WorkerProcess();

    int Run();
    void SetUnixStreamSocket(std::shared_ptr<UnixStreamSocket> channel);
    int SetListener(Listener *listener);

public:
    void ReceiveMessage(std::string &payload);

public:
    void OnWorkerExited(uv_process_t *req, int64_t status, int termSignal);

protected:
    int SetPipe();
    int Spawn();

// PipeServer listener
public:
    virtual void OnChannelAccept(UnixStreamSocket *channel) override;
    virtual void OnChannelClosed(UnixStreamSocket *channel) override;
    virtual void OnChannelRecv(UnixStreamSocket *channel, std::string_view &payload) override;

private:
    uv_process_t         m_process;
    uv_process_options_t m_options;
    Options              m_opt;
    PipeClient          *m_pipeClient { nullptr };
    PipeServer          *m_pipeServer { nullptr };
    UnixStreamSocket    *m_channel { nullptr };
    std::string          m_pipeFile;
};

}
