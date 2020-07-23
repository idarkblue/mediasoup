#ifndef _HTTP_SERVER_HPP_INCLUDE_
#define _HTTP_SERVER_HPP_INCLUDE_

#include <string>
#include <map>
#include <list>
#include "App.h"
#include "MessageListener.hpp"
#include "MasterRequest.hpp"

namespace Master {

class HttpServer {

public:
    HttpServer(MessageListener *listener = nullptr);
    virtual ~HttpServer();


    int SetHttp(int port, MessageListener *listener = nullptr);
    int SetHttp(int port, std::string keyfile, std::string certfile, std::string passphrase, MessageListener *listener = nullptr);

    int OnPost(void *handle, uWS::HttpRequest *req, std::string_view chunk, bool isEnd, bool ssl);
    void OnAborted(void *handle);

protected:
    void RemoveRequest(void *handle);
    MasterRequest* FetchRequest(void *handle, bool ssl);

private:
    MasterRequest* GetRequestSpace(bool ssl);
    void PutRequestSpace(MasterRequest *r);

private:
    int m_httpPort  { 80 };
    int m_httpsPort { 443 };

    std::map<void *, MasterRequest *> m_requestMap;
    uWS::App        *m_httpApp  { nullptr };
    uWS::SSLApp     *m_httpsApp { nullptr };
    MessageListener *m_listener { nullptr };

    std::list<MasterRequest*> m_freeRequest;
};

}

#endif
