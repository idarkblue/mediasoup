#pragma once
#include <string>
 
extern "C"
{
    #include "uv.h"
}

namespace Master {

class PipeClient;
 
class PipeClient
{
public:
    struct UvPipe {
        uv_pipe_t   pipe;
        PipeClient *client;
    };

public:
    PipeClient(uv_loop_t *loop);
    ~PipeClient();

    int Connect(std::string pipeFile);
    uv_pipe_t* GetPipeHandle();

public:
    void OnConnection(uv_connect_t* req,int status);
    void Write(uv_write_t* req,int status);

public:
    uv_loop_t    *m_loop { nullptr };
    UvPipe        m_pipeClient;
    uv_connect_t *m_pipeConnect;
};
}
