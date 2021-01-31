#include "NetServer.hpp"

namespace pingos {

    NetConnection::NetConnection(void *handler) : handler(handler)
    {

    }

    NetConnection::~NetConnection()
    {

    }

    int NetConnection::AppendReceiveBuffer(std::string_view data)
    {
        this->receiveBuffer += data;

        return 0;
    }

    std::string NetConnection::GetReceiveBuffer()
    {
        return this->receiveBuffer;
    }

    void NetConnection::ClearReceiveBuffer()
    {
        this->receiveBuffer = "";

        return;
    }

    void* NetConnection::GetHandler()
    {
        return this->handler;
    }

    void NetConnection::SetHandler(void *handler)
    {
        this->handler = handler;
    }

    void NetConnection::SetContext(void *ctx)
    {
        this->ctx = ctx;
    }

    void* NetConnection::GetContext()
    {
        return this->ctx;
    }

    void NetConnection::SetServer(NetServer *svr)
    {
        this->svr = svr;
    }

    void* NetConnection::GetServer()
    {
        return this->svr;
    }

    // class NetConnection end

    // class NetServer begin
    NetServer::NetServer()
    {

    }

    NetServer::~NetServer()
    {

    }

    void NetServer::SetListener(NetServer::Listener *listener)
    {
        this->listener = listener;
    }

    NetConnection* NetServer::FindConnection(void *handler)
    {
        auto it = this->ncMap.find(handler);
        if (it == this->ncMap.end()) {
            return nullptr;
        }

        return it->second;
    }

    NetConnection* NetServer::FetchConnection(void *handler)
    {
        auto it = this->ncMap.find(handler);
        if (it != this->ncMap.end()) {
            return it->second;
        }

        auto nc = this->NewConnection(handler);
        this->ncMap[handler] = nc;

        return nc;
    }

    void NetServer::RemoveConnection(void *handler)
    {
        auto it = this->ncMap.find(handler);
        if (it != this->ncMap.end() && it->second) {
            this->DeleteConnection(it->second);
        }
        this->ncMap.erase(handler);
    }

}
