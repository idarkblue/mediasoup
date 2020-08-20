#define PMS_CLASS "HttpServer"
#include <string>
#include "Master/Log.hpp"
#include "Master/HttpServer.hpp"

namespace Master {
HttpServer::HttpServer(NetServer::Listener *listener)
{
    m_listener = listener;
}

HttpServer::~HttpServer()
{

}

int HttpServer::StartHttp(int port)
{
    us_socket_context_options_t options;
    m_httpPort = port;
    m_httpApp = new uWS::App(options);
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
            PMS_INFO("Listening on port {}", port);
        } else {
            PMS_ERROR("Listening on port {} failed.", port);
        }
    });

    return 0;
}

int HttpServer::StartHttp(int port, std::string keyfile, std::string certfile, std::string passphrase)
{
    m_httpsPort = port;

    us_socket_context_options_t opt;
    opt.key_file_name = keyfile.c_str();
    opt.cert_file_name = certfile.c_str();
    opt.passphrase = passphrase.c_str();

    m_httpsApp = new uWS::SSLApp(opt);

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
            PMS_ERROR("Listening on port {} success", port);
        } else {
            PMS_ERROR("Listening on port {} failed.", port);
        }
    });

    return 0;
}

int HttpServer::ReplyBinary(NetRequest *request, const uint8_t *nsPayload, size_t nsPayloadLen)
{
    if (request == nullptr || request->GetConnectionHandle() == nullptr) {
        return -1;
    }

    if (request->IsSsl()) {
        uWS::HttpResponse<true> *res = (uWS::HttpResponse<true>*) request->GetConnectionHandle();

        std::string content((const char *)nsPayload, nsPayloadLen);
        res->end(content);
    } else {
        uWS::HttpResponse<false> *res = (uWS::HttpResponse<false>*) request->GetConnectionHandle();

        std::string content((const char *)nsPayload, nsPayloadLen);
        res->end(content);
    }

    return 0;
}

int HttpServer::ReplyString(NetRequest *request, std::string data)
{
    if (request == nullptr || request->GetConnectionHandle() == nullptr) {
        return -1;
    }

    if (request->IsSsl()) {
        uWS::HttpResponse<true> *res = (uWS::HttpResponse<true>*) request->GetConnectionHandle();

        res->end(data);
    } else {
        uWS::HttpResponse<false> *res = (uWS::HttpResponse<false>*) request->GetConnectionHandle();

        res->end(data);
    }

    return 0;
}

int HttpServer::OnPost(void *handle, uWS::HttpRequest *req, std::string_view chunk, bool isEnd, bool ssl)
{
    NetRequest *request = this->FetchRequest(handle, ssl);

    if (request == nullptr) {
        PMS_ERROR("Fetch request failed.");
        return -1;
    }

    request->Padding(chunk);

    if (isEnd) {
        if (request->ParseUri(req->getUrl()) != 0) {
            PMS_ERROR("Failed to parse uri {}", req->getUrl());
            return -1;
        }

        if (m_listener) {
            m_listener->OnMessage(request);
        }
    }

    return 0;
}

void HttpServer::OnAborted(void *handle)
{
    auto it = m_requestMap.find(handle);
    if (it != m_requestMap.end() && this->m_listener) {
        m_listener->OnAborted(it->second);
    }

    this->RemoveRequest(handle);
}

}
