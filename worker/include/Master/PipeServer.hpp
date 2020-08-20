#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "UnixStreamSocket.hpp"

extern "C"
{
    #include "uv.h"
}

namespace Master {
class PipeServer;
 
class PipeServer: public UnixStreamSocket::Listener {
public:
    typedef struct {
        uv_pipe_t pipe;
        PipeServer *server;
    } UvPipe;

public:
    class Listener {
    public:
        virtual void OnChannelAccept(PipeServer *ps, UnixStreamSocket *channel) = 0;
        virtual void OnChannelClosed(PipeServer *ps, UnixStreamSocket *channel) = 0;
        virtual void OnChannelRecv(PipeServer *ps, UnixStreamSocket *channel, std::string_view &payload) = 0;
    };

public:
    PipeServer(uv_loop_t* loop);
    virtual ~PipeServer();

public:
    virtual void OnChannelMessage(Master::UnixStreamSocket* channel, std::string_view &payload) override;
    virtual void OnChannelClosed(Master::UnixStreamSocket* channel) override;

public:
    int Listen(std::string pipeName);
    std::string GetPipeName() { return m_pipeName; };
    void SetListener(Listener *listener);
    void OnAccept(uv_stream_t* server,int status);
    void OnRead(uv_stream_t* client,ssize_t nread,const uv_buf_t* buf);
    void AllocBuffer(uv_handle_t* handle,size_t suggested_size,uv_buf_t* buf);

private:
    std::string   m_pipeName;
    UvPipe        m_pipeServer;
    uv_loop_t    *m_loop;
    Listener     *m_listener;
    std::vector<UnixStreamSocket *> m_clients;
};
}
