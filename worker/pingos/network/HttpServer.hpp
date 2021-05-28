#pragma once

#include <uv.h>
#include "App.h"
#include "NetServer.hpp"

namespace pingos {

    class HttpServer;
    class HttpConnection;

    class HttpServer : public NetServer {
    public:
        HttpServer();
        virtual ~HttpServer();

    public:
        class HttpConnection : public NetConnection {
        public:
            HttpConnection() = default;
            HttpConnection(bool ssl);
            virtual ~HttpConnection() = default;

        public:
            void SetServer(HttpServer *server);
            void SetSSL(bool ssl);
            virtual int Send(std::string &data);

        public:
            HttpConnection *next { nullptr };

        private:
            HttpServer *server;
        };

    public:
        static void ClassInit(uv_loop_t * loop);
        int Start(std::string ip, uint16_t port, std::string pattern);
        int Start(std::string ip, uint16_t port, std::string pattern,
            std::string keyfile, std::string certfile, std::string passphrase);

    protected:
        NetConnection* NewConnection(bool ssl) override;
        void DeleteConnection(NetConnection*) override;

    private:
        uWS::App    *app  { nullptr };
        uWS::SSLApp *sslApp { nullptr };

        HttpConnection *freeConnections { nullptr };
    };

}
