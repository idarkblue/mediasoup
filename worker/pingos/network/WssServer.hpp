#pragma once

#include <string>
#include <map>
#include <list>
#include "App.h"
#include "NetServer.hpp"

namespace pingos {
    class WssServer;
    class WssServer : public NetServer {
    public:
        class WssConnection;
        class WssConnection : public NetConnection {
        public:
            WssConnection() = default;
            WssConnection(bool ssl);
            virtual ~WssConnection() = default;

        public:
            void SetServer(WssServer *server);
            void SetSSL(bool ssl);
            virtual int Send(std::string &data);

        public:
            WssConnection *next { nullptr };
            WssServer *server { nullptr };
        };

    public:
        WssServer(uWS::CompressOptions compress,
                int maxPayloadLength,
                int idleTimeout,
                int maxBackpressure);
        virtual ~WssServer();

    protected:
        void OnDrain(NetConnection *nc);
        void OnMessage(NetConnection *nc, std::string_view message, uWS::OpCode opCode);
        void OnPing(NetConnection *nc);
        void OnPong(NetConnection *nc);
        void OnClose(NetConnection *nc, int code, std::string_view message);

    private:
        void Close();

    public:
        int Start(std::string ip, uint16_t port, std::string pattern);
        int Start(std::string ip, uint16_t port, std::string pattern,
            std::string keyfile, std::string certfile, std::string passphrase);

    protected:
        NetConnection* NewConnection(bool ssl) override;
        void DeleteConnection(NetConnection*) override;

    private:
        uWS::App    *app  { nullptr };
        uWS::SSLApp *sslApp { nullptr };
        us_listen_socket_t *listenSocket { nullptr };
        uWS::CompressOptions compression;
        int maxPayloadLength;
        int idleTimeout;
        int maxBackpressure;

        WssConnection *freeConnections { nullptr };
    };
}
