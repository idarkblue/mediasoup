#define PMS_CLASS "pingos::HttpServer"
#include <string>
#include "Master/Log.hpp"
#include "Master/HttpServer.hpp"

namespace pingos {
HttpServer::HttpServer(NetServer::Listener *listener)
{
    m_listener = listener;
}

HttpServer::~HttpServer()
{

}

int HttpServer::Accept(uint16_t port)
{
    us_socket_context_options_t options;
    m_port = port;
    m_app = new uWS::App(options);
    m_app->post("/*", [this](uWS::HttpResponse<false> *res, uWS::HttpRequest *req) {
        res->onAborted([this, res]() {
            this->OnDisconnect(res);
        });

        res->onData([this, res, req](std::string_view chunk, bool isEnd) {
            if (this->OnPost(res, req, chunk, isEnd, false) == -1) {
               res->end("ServerError");
            }
        });
    });

    m_app->listen(port, [port](us_listen_socket_t *listenSocket) {
        if (listenSocket) {
            PMS_INFO("Listening on port {}", port);
        } else {
            PMS_ERROR("Listening on port {} failed.", port);
        }
    });

    return 0;
}

int HttpServer::Accept(uint16_t port, std::string keyfile, std::string certfile, std::string passphrase)
{
    m_sslPort = port;

    us_socket_context_options_t opt;
    opt.key_file_name = keyfile.c_str();
    opt.cert_file_name = certfile.c_str();
    opt.passphrase = passphrase.c_str();

    m_sslApp = new uWS::SSLApp(opt);

    m_sslApp->post("/*", [this](uWS::HttpResponse<true> *res, uWS::HttpRequest *req) {
        res->onAborted([this, res]() {
            this->OnDisconnect(res);
        });

        res->onData([this, res, req](std::string_view chunk, bool isEnd) {
            if (this->OnPost(res, req, chunk, isEnd, true) == -1) {
               res->end("ServerError");
            }
        });
    });

    m_sslApp->listen(port, [port](us_listen_socket_t *listenSocket) {
        if (listenSocket) {
            PMS_ERROR("Listening on port {} success", port);
        } else {
            PMS_ERROR("Listening on port {} failed.", port);
        }
    });

    return 0;
}

int HttpServer::Disconnect(NetConnection *nc)
{
    return 0;
}

int HttpServer::ReplyBinary(NetConnection *nc, const uint8_t *nsPayload, size_t nsPayloadLen)
{
    if (nc == nullptr || nc->GetConnectionHandler() == nullptr) {
        return -1;
    }

    if (nc->IsSsl()) {
        uWS::HttpResponse<true> *res = (uWS::HttpResponse<true>*) nc->GetConnectionHandler();

        std::string content((const char *)nsPayload, nsPayloadLen);
        res->end(content);
    } else {
        uWS::HttpResponse<false> *res = (uWS::HttpResponse<false>*) nc->GetConnectionHandler();

        std::string content((const char *)nsPayload, nsPayloadLen);
        res->end(content);
    }

    return 0;
}

int HttpServer::ReplyString(NetConnection *nc, std::string data)
{
    if (nc == nullptr || nc->GetConnectionHandler() == nullptr) {
        return -1;
    }

    if (nc->IsSsl()) {
        uWS::HttpResponse<true> *res = (uWS::HttpResponse<true>*) nc->GetConnectionHandler();

        res->end(data);
    } else {
        uWS::HttpResponse<false> *res = (uWS::HttpResponse<false>*) nc->GetConnectionHandler();

        res->end(data);
    }

    return 0;
}

int HttpServer::OnPost(void *handle, uWS::HttpRequest *req, std::string_view chunk, bool isEnd, bool ssl)
{
    NetConnection *nc = this->FetchConnection(handle, ssl);

    if (nc == nullptr) {
        PMS_ERROR("Fetch nc failed.");
        return -1;
    }

    nc->Padding(chunk);

    if (isEnd) {
        if (m_listener) {
            m_listener->OnMessage(nc);
        }
    }

    return 0;
}

void HttpServer::OnDisconnect(void *handle)
{
    auto it = m_ncMap.find(handle);
    if (it != m_ncMap.end() && this->m_listener) {
        m_listener->OnDisconnect(it->second);
        m_listener = nullptr;
    }

    this->RecycleConnection(handle);
}

}
