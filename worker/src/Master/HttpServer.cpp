#define PMS_CLASS "HttpServer"
#include <string>
#include "Master/Log.hpp"
#include "Master/HttpServer.hpp"

namespace Master {
HttpServer::HttpServer(MessageListener *listener)
{
    m_listener = listener;
}

HttpServer::~HttpServer()
{

}

int HttpServer::SetHttp(int port, MessageListener *listener)
{
    if (listener) {
        m_listener = listener;
    }
    m_httpPort = port;
    m_httpApp = new uWS::App();
    m_httpApp->post("/*", [this](uWS::HttpResponse<false> *res, uWS::HttpRequest *req) {
        res->onAborted([this, res]() {
            this->OnAborted(res);
        });

        res->onData([this, res, req](std::string_view chunk, bool isEnd) {
            if (this->OnPost(res, req, chunk, isEnd, false) == -1) {
               res->end("ServerError");
            }
        });
    });

    m_httpApp->listen(port, [port](us_listen_socket_t *listenSocket) {
        if (listenSocket) {
            PMS_INFO("Listening on port {}\r\n", port);
        } else {
            PMS_ERROR("Listening on port {} failed.\r\n", port);
        }
    });

    return 0;
}

int HttpServer::SetHttp(int port, std::string keyfile, std::string certfile, std::string passphrase, MessageListener *listener)
{
    if (listener) {
        m_listener = listener;
    }
    m_httpsPort = port;

    m_httpsApp = new uWS::SSLApp({
        .key_file_name = keyfile.c_str(),
        .cert_file_name = certfile.c_str(),
        .passphrase = passphrase.c_str(),
    });

    m_httpsApp->post("/*", [this](uWS::HttpResponse<true> *res, uWS::HttpRequest *req) {
        res->onAborted([this, res]() {
            this->OnAborted(res);
        });

        res->onData([this, res, req](std::string_view chunk, bool isEnd) {
            if (this->OnPost(res, req, chunk, isEnd, true) == -1) {
               res->end("ServerError");
            }
        });
    });

    m_httpsApp->listen(port, [port](us_listen_socket_t *listenSocket) {
        if (listenSocket) {
            PMS_ERROR("Listening on port {} success\r\n", port);
        } else {
            PMS_ERROR("Listening on port {} failed.\r\n", port);
        }
    });

    return 0;
}

int HttpServer::OnPost(void *handle, uWS::HttpRequest *req, std::string_view chunk, bool isEnd, bool ssl)
{
    MasterRequest *r = this->FetchRequest(handle, ssl);

    if (r == nullptr) {
        PMS_ERROR("Fetch request failed.");
        return -1;
    }

    r->Padding(chunk);

    if (isEnd) {
        r->SetUri(req->getUrl());
        if (m_listener) {
            m_listener->OnMessage(r);
        }
    }

    return 0;
}

void HttpServer::OnAborted(void *handle)
{
    this->RemoveRequest(handle);
}

void HttpServer::RemoveRequest(void *handle)
{
    auto it = m_requestMap.find(handle);
    if (it == m_requestMap.end()) {
        return;
    }

    MasterRequest *r = it->second;

    if (r != nullptr && m_listener) {
        m_listener->OnAborted(r);
    }

    m_requestMap.erase(it);

    this->PutRequestSpace(r);
}

MasterRequest* HttpServer::FetchRequest(void *handle, bool ssl)
{
    auto it = m_requestMap.find(handle);
    if (it != m_requestMap.end()) {
        return it->second;
    }

    auto r = this->GetRequestSpace(ssl);
    m_requestMap[handle] = r;

    r->Reset(MessageSourceType::MS_HTTP, ssl, handle, nullptr);

    return r;
}

MasterRequest* HttpServer::GetRequestSpace(bool ssl)
{
    if (m_freeRequest.size() == 0) {
        return new MasterRequest();
    }

    MasterRequest *r = m_freeRequest.front();

    m_freeRequest.pop_front();

    return r;

}

void HttpServer::PutRequestSpace(MasterRequest *r)
{
    r->Clear();
    m_freeRequest.push_back(r);
}

}
