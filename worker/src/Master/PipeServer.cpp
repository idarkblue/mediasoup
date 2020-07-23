#define PMS_CLASS "Master::PipeServer"

#include "Master/PipeServer.hpp"
#include "Master/Log.hpp"

namespace Master {

uv_loop_t *m_loop = nullptr;

static void onPipeAccept(uv_stream_t* server,int status)
{
    PipeServer::UvPipe *pipe = (PipeServer::UvPipe*) server;
    pipe->server->OnAccept(server, status);
}

PipeServer::PipeServer(uv_loop_t* loop)
{
    m_loop = loop;
}

PipeServer::~PipeServer()
{
}

int PipeServer::Listen(std::string pipeName)
{
    m_pipeName = pipeName;

    uv_fs_t req;
    uv_fs_unlink(m_loop, &req, pipeName.c_str(), NULL);
    uv_pipe_init(m_loop, (uv_pipe_t*)&m_pipeServer, 0);

    m_pipeServer.server = this;

    int ret = uv_pipe_bind((uv_pipe_t*)&m_pipeServer, pipeName.c_str());
    if(ret) {
        PMS_ERROR("Bind error {}", uv_err_name(ret));
        return -1;
    }

    ret = uv_listen((uv_stream_t*)&m_pipeServer, 128, onPipeAccept);
    if (ret) {
        PMS_ERROR("Listen error {}", uv_err_name(ret));
        return -1;
    }

    return 0;

}

void PipeServer::SetListener(Listener *listener)
{
    m_listener = listener;
}

/*
void PipeServer::remove_sock(int sig)
{
    uv_fs_t req;
    uv_loop_t* m_loop=uv_default_loop();
    uv_fs_unlink(m_loop,&req,PIPENAME,NULL);
    exit(0);
}
*/
void PipeServer::OnAccept(uv_stream_t* server, int status)
{
    if (status < 0) {
        PMS_ERROR("New pipe connection error...");
        return;
    }

    uv_pipe_t* pipe = (uv_pipe_t*) malloc(sizeof(uv_pipe_t));

    uv_pipe_init(m_loop, pipe, 0);

    int ret = uv_accept(server, (uv_stream_t*)pipe);
    if (ret != 0) {
        PMS_ERROR("Accept failed, {} ...", uv_err_name(ret));
        return;
    }

    PMS_INFO("Accept success...");

    auto client = new Master::UnixStreamSocket(pipe, this);

    if (m_listener) {
        m_listener->OnChannelAccept(client);
    }

    m_clients.push_back(client);
}

void PipeServer::OnChannelMessage(Master::UnixStreamSocket* channel, std::string_view &payload)
{
    if (m_listener) {
        m_listener->OnChannelRecv(channel, payload);
    }

    PMS_INFO("{}", payload);
}

void PipeServer::OnChannelClosed(Master::UnixStreamSocket* channel)
{
    if (m_listener) {
        m_listener->OnChannelClosed(channel);
    }

    PMS_ERROR("channel closed");
}

}