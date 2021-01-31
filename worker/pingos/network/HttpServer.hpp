#pragma once

#include <string>
#include <map>
#include <list>
#include "App.h"
#include "NetServer.hpp"

namespace pingos {

    class HttpConnection;

    class HttpConnection : public NetConnection {
    public:
        HttpConnection(void *handler);
        virtual ~HttpConnection();

    public:
        void SetSSL();

    public:
        // pingos::NetConnetion
        int ReplyBinary(const uint8_t *nsPayload, size_t nsPayloadLen) override;
        int ReplyString(std::string data) override;

    public:
        HttpConnection *next { nullptr };

    private:
        bool ssl { false };
    };

    class HttpServer : public NetServer {

    public:
        HttpServer(Listener *listener = nullptr);
        virtual ~HttpServer();

        int OnPost(void *handle, uWS::HttpRequest *req, std::string_view chunk, bool isEnd, bool ssl);
        void OnDisconnect(void *handle);

    public:
        int Accept(std::string ip, uint16_t port, std::string location);
        int Accept(std::string ip, uint16_t port, std::string location, std::string keyfile, std::string certfile, std::string passphrase);

    protected:
        NetConnection* NewConnection(void *handler) override;
        void DeleteConnection(NetConnection *nc) override;

    private:
        uWS::App        *app  { nullptr };
        uWS::SSLApp     *sslApp { nullptr };

        HttpConnection *freeConnections { nullptr };
    };

}
