#include "NetServer.hpp"

namespace pingos {

    NetServer::NetServer()
    {

    }

    NetServer::~NetServer()
    {

    }

    void NetServer::SetListener(Listener *listener)
    {
        this->listener = listener;
    }

    int NetServer::JoinConnection(void *handle, bool ssl)
    {
        auto c = this->NewConnection(ssl);
        if (!c) {
            return -1;
        }

        c->SetHandle(handle);

        this->ncMap[handle] = c;

        if (this->listener) {
            this->listener->OnNetConnected(c);
        }

        return 0;
    }

    void NetServer::RemoveConnection(void *handle)
    {
        auto it = this->ncMap.find(handle);
        if (it == this->ncMap.end()) {
            return;
        }

        if (this->listener) {
            this->listener->OnNetDisconnected(it->second);
        }

        this->DeleteConnection(it->second);

        this->ncMap.erase(it);
    }

    NetConnection* NetServer::FetchConnection(void *handle, bool ssl)
    {
        auto it = this->ncMap.find(handle);
        if (it == this->ncMap.end()) {
            JoinConnection(handle, ssl);
        }

        it = this->ncMap.find(handle);
        if (it == this->ncMap.end()) {
            return nullptr;
        }

        return it->second;
    }

    NetConnection* NetServer::FindConnection(void *handle)
    {
        auto it = this->ncMap.find(handle);
        if (it == this->ncMap.end()) {
            return nullptr;
        }

        return it->second;
    }


}
