#ifndef _HTTP_SERVER_HPP_INCLUDE_
#define _HTTP_SERVER_HPP_INCLUDE_

#include <string>
#include <map>
#include <list>
#include "App.h"
#include "NetServer.hpp"

namespace Master {

class HttpServer : public NetServer {

public:
    HttpServer(Listener *listener = nullptr);
    virtual ~HttpServer();

    void SetListener(Listener *listener);
    int StartHttp(int port);
    int StartHttp(int port, std::string keyfile, std::string certfile, std::string passphrase);

    int OnPost(void *handle, uWS::HttpRequest *req, std::string_view chunk, bool isEnd, bool ssl);
    void OnAborted(void *handle);

public:
    virtual int ReplyBinary(NetRequest *request, const uint8_t *nsPayload, size_t nsPayloadLen) override;
    virtual int ReplyString(NetRequest *request, std::string data) override;

private:
    int m_httpPort  { 80 };
    int m_httpsPort { 443 };

    uWS::App        *m_httpApp  { nullptr };
    uWS::SSLApp     *m_httpsApp { nullptr };
};

}

#endif
