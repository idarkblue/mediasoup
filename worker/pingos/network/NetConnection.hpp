#pragma once

#include <string>

namespace pingos {
    class NetConnection {
    public:
        NetConnection();
        NetConnection(bool bssl);
        virtual ~NetConnection();

    public:
        void AppendRecvBuffer(std::string &data);
        std::string GetRecvBuffer();
        void ClearRecvBuffer();

    public:
        virtual int Send(std::string &data) = 0;

    public:
        void SetContext(void *ctx);
        void* GetContext();

    public:
        void SetHandle(void *handle);
        void* GetHandle();

    public:
        bool  ssl {false};
        
    private:
        std::string recvBuffer { "" };
        void* handle { nullptr };
        void* context { nullptr };

    };
}
