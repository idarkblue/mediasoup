#ifndef _HTTP_SERVER_HPP_INCLUDE_
#define _HTTP_SERVER_HPP_INCLUDE_

#include <string>
#include <map>
#include <list>
#include "App.h"
#include "NetServer.hpp"

namespace pingos {

class HttpServer : public NetServer {

public:
    HttpServer(Listener *listener = nullptr);
    virtual ~HttpServer();

    int OnPost(void *handle, uWS::HttpRequest *req, std::string_view chunk, bool isEnd, bool ssl);
    void OnDisconnect(void *handle);

public:
    virtual int Accept(std::string ip, uint16_t port, std::string location) override;
    virtual int Accept(std::string ip, uint16_t port, std::string location, std::string keyfile, std::string certfile, std::string passphrase) override;
    virtual int Disconnect(NetConnection *nc) override;
    virtual int ReplyBinary(NetConnection *nc, const uint8_t *nsPayload, size_t nsPayloadLen) override;
    virtual int ReplyString(NetConnection *nc, std::string data) override;

private:
    uWS::App        *app  { nullptr };
    uWS::SSLApp     *sslApp { nullptr };
};

}

#endif
