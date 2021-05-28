#pragma once

#include <list>
#include <map>
#include <string>
#include <string_view>
#include "NetConnection.hpp"

namespace pingos {

    class NetServer;
    class NetServer {

    public:
        class Listener {
        public:
            virtual ~Listener() = default;
            virtual void OnNetConnected(NetConnection *nc) = 0;
            virtual void OnNetDataReceived(NetConnection *nc) = 0;
            virtual void OnNetDisconnected(NetConnection *nc) = 0;
        };

    public:
        NetServer();
        virtual ~NetServer();

        void SetListener(NetServer::Listener *listener);

    protected:
        virtual NetConnection* NewConnection(bool ssl) = 0;
        virtual void DeleteConnection(NetConnection *nc) = 0;

    public:
        int JoinConnection(void *handle, bool ssl);
        void RemoveConnection(void *handle);
        NetConnection* FetchConnection(void *handle, bool ssl);
        NetConnection* FindConnection(void *handle);

    protected:
        std::map<void *, NetConnection *> ncMap;
        Listener                         *listener { nullptr };
    };

}
