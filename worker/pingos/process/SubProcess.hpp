#pragma once

#include <string>
#include <iostream>
#include <vector>
#include "UnixStreamSocket.hpp"
#include "Pipe.hpp"

#include <uv.h>

namespace pingos {

class SubProcess : public UnixStreamSocket::Listener
{
public:
    class Listener {
    public:
        virtual void OnProcessExited(SubProcess *w) = 0;
    };

public:
    SubProcess(uv_loop_t *loop);
    virtual ~SubProcess();

    int Start(int slot, std::string file);
    int SetListener(Listener *listener);

public:
    void OnProcessExited(uv_process_t *req, int64_t status, int termSignal);

// UnixStreamSocket listener
public:
    virtual void OnChannelMessage(pingos::UnixStreamSocket* channel, std::string_view &payload) override;
    virtual void OnChannelClosed(pingos::UnixStreamSocket* channel) override;

protected:
    int InitChannels();
    int Spawn();
    int ChannelSend(std::string data);

public:
    virtual void OnProcessMessage(std::string_view &payload) = 0;

private:
    uv_process_options_t options;
    uv_process_t         process;
    Listener            *listener {nullptr};
    UnixStreamSocket    *channelIn { nullptr };
    UnixStreamSocket    *channelOut { nullptr };
    UnixStreamSocket    *payloadChannelIn { nullptr };
    UnixStreamSocket    *payloadChannelOut { nullptr };

    uv_loop_t           *loop { nullptr };
    Pipe                 channelPipe;
    Pipe                 payloadChannelPipe;

    char               **args { nullptr };
    int                  slot { -1 };
    std::string          file { "" };

    uv_stdio_container_t childStdio[7];
    char *env[2];
};

}
