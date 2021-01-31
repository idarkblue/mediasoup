#pragma once

#include <list>
#include <map>
#include <string>
#include <string_view>

namespace pingos {

    class NetServer;
    class NetConnection;

    class NetConnection {
    public:
        NetConnection() = default;
        NetConnection(void *handler);
        virtual ~NetConnection();

    public:
        int AppendReceiveBuffer(std::string_view data);
        std::string GetReceiveBuffer();
        void ClearReceiveBuffer();

    public:
        virtual int ReplyBinary(const uint8_t *nsPayload, size_t nsPayloadLen) = 0;
        virtual int ReplyString(std::string data) = 0;

    public:
        void SetHandler(void *handler);
        void *GetHandler();

        void SetContext(void *s);
        void* GetContext();

        void SetServer(NetServer *svr);
        void* GetServer();

    protected:
        void *handler { nullptr };
        void *ctx { nullptr };
        std::string receiveBuffer { "" };
        NetServer *svr;
    };

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
        virtual NetConnection* NewConnection(void *handler) = 0;
        virtual void DeleteConnection(NetConnection *nc) = 0;

    public:
        NetConnection* FindConnection(void *handler);
        NetConnection* FetchConnection(void *handler);
        void RemoveConnection(void *handler);

    protected:
        std::map<void *, NetConnection *> ncMap;
        Listener                         *listener { nullptr };
    };

}
