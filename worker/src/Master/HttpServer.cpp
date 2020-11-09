#define PMS_CLASS "pingos::HttpServer"
#include <string>
#include "Master/Log.hpp"
#include "Master/HttpServer.hpp"

namespace pingos {
HttpServer::HttpServer(NetServer::Listener *listener)
{
    this->listener = listener;
}

HttpServer::~HttpServer()
{

}

int HttpServer::Accept(std::string ip, uint16_t port, std::string location)
{
    us_socket_context_options_t options;
    this->port = port;
    this->app = new uWS::App(options);
    this->app->post(location, [this](uWS::HttpResponse<false> *res, uWS::HttpRequest *req) {
        res->onAborted([this, res]() {
            this->OnDisconnect(res);
        });

        res->onData([this, res, req](std::string_view chunk, bool isEnd) {
            if (this->OnPost(res, req, chunk, isEnd, false) == -1) {
               res->end("ServerError");
            }
        });
    }).listen(ip, port, [ip, port](us_listen_socket_t *listenSocket) {
        if (listenSocket) {
            PMS_INFO("Listening http port {}:{} success", ip, port);
        } else {
            PMS_ERROR("Listening http port {}:{} failed.", ip, port);
        }
    });

    return 0;
}

int HttpServer::Accept(std::string ip, uint16_t port, std::string location, std::string keyfile, std::string certfile, std::string passphrase)
{
    this->sslPort = port;

    this->sslApp = new uWS::SSLApp({
        .key_file_name = keyfile.c_str(),
        .cert_file_name = certfile.c_str(),
        .passphrase = passphrase.c_str()
    });

    this->sslApp->post(location, [this](uWS::HttpResponse<true> *res, uWS::HttpRequest *req) {
        res->onAborted([this, res]() {
            this->OnDisconnect(res);
        });

        res->onData([this, res, req](std::string_view chunk, bool isEnd) {
            if (this->OnPost(res, req, chunk, isEnd, true) == -1) {
               res->end("ServerError");
            }
        });
    }).listen(ip, port, [ip, port](us_listen_socket_t *listenSocket) {
        if (listenSocket) {
            PMS_INFO("Listening https port {}:{} success", ip, port);
        } else {
            PMS_ERROR("Listening https port {}:{} failed.", ip, port);
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
        if (this->listener) {
            this->listener->OnMessage(nc);
        }
    }

    return 0;
}

void HttpServer::OnDisconnect(void *handle)
{
    auto it = this->ncMap.find(handle);
    if (it != this->ncMap.end() && this->listener) {
        this->listener->OnDisconnect(it->second);
        this->listener = nullptr;
    }

    this->RecycleConnection(handle);
}

}
