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
    virtual int Accept(uint16_t port) override;
    virtual int Accept(uint16_t port, std::string keyfile, std::string certfile, std::string passphrase) override;
    virtual int Disconnect(NetConnection *nc) override;
    virtual int ReplyBinary(NetConnection *nc, const uint8_t *nsPayload, size_t nsPayloadLen) override;
    virtual int ReplyString(NetConnection *nc, std::string data) override;

private:
    int m_port  { 80 };
    int m_sslPort { 443 };

    uWS::App        *m_app  { nullptr };
    uWS::SSLApp     *m_sslApp { nullptr };
};

}

#endif
