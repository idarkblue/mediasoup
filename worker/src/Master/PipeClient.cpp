#define PMS_CLASS "pingos::PipeClient"

#include "Master/PipeClient.hpp"
#include "Master/Log.hpp"

namespace pingos {

static void onPipeConnection(uv_connect_t* req, int status)
{
    PipeClient::UvPipe* pipe = (PipeClient::UvPipe*) req->handle;
    PipeClient* client = pipe->client;
    client->OnConnection(req, status);
}

PipeClient::PipeClient(uv_loop_t *loop)
{
    m_loop = loop;
}

PipeClient::~PipeClient()
{
}

int PipeClient::Connect(std::string pipeFile)
{
    m_pipeClient.client = this;

    uv_pipe_init(m_loop, (uv_pipe_t*) &m_pipeClient, 0);
    m_pipeConnect = (uv_connect_t*) malloc(sizeof(uv_connect_t));
    uv_pipe_connect(m_pipeConnect, (uv_pipe_t*) &m_pipeClient, pipeFile.c_str(), onPipeConnection);

    return 0;
}

uv_pipe_t* PipeClient::GetPipeHandle()
{
    return &m_pipeClient.pipe;
}
 
void PipeClient::OnConnection(uv_connect_t* req,int status)
{

    if (status < 0) {
        PMS_ERROR("New conect error...");
    }
}

void PipeClient::Write(uv_write_t* req, int status)
{
    if (status < 0) {
        PMS_ERROR("Write error...");
    }
}

}
