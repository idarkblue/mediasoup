#include "NetConnection.hpp"
#include "Log.hpp"

namespace pingos {
    NetConnection::NetConnection()
    {

    }
    NetConnection::NetConnection(bool ssl):ssl(ssl)
    {

    }

    NetConnection::~NetConnection()
    {

    }

    void NetConnection::AppendRecvBuffer(std::string &data)
    {
        this->recvBuffer += data;
    }

    std::string NetConnection::GetRecvBuffer()
    {
        return this->recvBuffer;
    }

    void NetConnection::ClearRecvBuffer()
    {
        this->recvBuffer.clear();
    }

    void NetConnection::SetContext(void *ctx)
    {
        this->context = ctx;
    }

    void* NetConnection::GetContext()
    {
        return this->context;
    }

    void NetConnection::SetHandle(void *handle)
    {
        this->handle = handle;
    }

    void* NetConnection::GetHandle()
    {
        return this->handle;
    }

}
