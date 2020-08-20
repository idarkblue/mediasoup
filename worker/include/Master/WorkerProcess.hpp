#pragma once

#include <string>
#include <iostream>
#include "PipeServer.hpp"
#include "PipeClient.hpp"
#include "UnixStreamSocket.hpp"
#include "WorkerRequest.hpp"

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
    void OnWorkerExited(uv_process_t *req, int64_t status, int termSignal);

protected:
    int SetPipe();
    int Spawn();

protected:
    void ProcessMessage(std::string_view &message);

// PipeServer listener
public:
    virtual void OnChannelAccept(PipeServer *ps, UnixStreamSocket *channel) override;
    virtual void OnChannelClosed(PipeServer *ps, UnixStreamSocket *channel) override;
    virtual void OnChannelRecv(PipeServer *ps, UnixStreamSocket *channel, std::string_view &payload) override;

public:
    int CreateRouter(const char *routerId);
    int CreateWebRtcTransport();
    int CreatePlainTransport();
    int CreateProducer();
    int CreateConsumer();

private:
    int Send2Process(ProcessRequest *request);

private:
    uv_process_t         m_process;
    uv_process_options_t m_options;
    Options              m_opt;
    PipeClient          *m_pipeClient[4] { nullptr, nullptr };
    UnixStreamSocket    *m_channel[4] { nullptr };
    PipeServer          *m_pipeServer[2] { nullptr };
    std::string          m_pipeFile;
    std::string          m_pipePayloadFile;
};

}
