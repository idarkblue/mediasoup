#define MS_CLASS "pingos::HttpServer"

#include <string>
#include "Logger.hpp"
#include "HttpServer.hpp"

namespace pingos {

    HttpConnection::HttpConnection(void *handler)
    {

    }

    HttpConnection::~HttpConnection()
    {

    }

    void HttpConnection::SetSSL()
    {
        this->ssl = true;
    }

    int HttpConnection::ReplyBinary(const uint8_t *nsPayload, size_t nsPayloadLen)
    {
        if (this->ssl) {
            uWS::HttpResponse<true> *res = (uWS::HttpResponse<true>*) this->handler;

            std::string content((const char *)nsPayload, nsPayloadLen);
            res->end(content);
        } else {
            uWS::HttpResponse<false> *res = (uWS::HttpResponse<false>*) this->handler;

            std::string content((const char *)nsPayload, nsPayloadLen);
            res->end(content);
        }

        if (this->svr) {
            this->svr->RemoveConnection(this->handler);
        }

        return 0;
    }

    int HttpConnection::ReplyString(std::string data)
    {
        if (this->ssl) {
            uWS::HttpResponse<true> *res = (uWS::HttpResponse<true>*) this->handler;

            res->end(data);
        } else {
            uWS::HttpResponse<false> *res = (uWS::HttpResponse<false>*) this->handler;

            res->end(data);
        }

        if (this->svr) {
            this->svr->RemoveConnection(this->handler);
        }

        return 0;
    }

    HttpServer::HttpServer(NetServer::Listener *listener)
    {
        this->listener = listener;
    }

    HttpServer::~HttpServer()
    {

    }

    int HttpServer::Accept(std::string ip, uint16_t port, std::string location)
    {
        this->app = new uWS::App();
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
                MS_INFO("Listening http port %s:%d success", ip.c_str(), port);
            } else {
                MS_ERROR("Listening http port %s:%d failed.", ip.c_str(), port);
            }
        });

        return 0;
    }

    int HttpServer::Accept(std::string ip, uint16_t port, std::string location, std::string keyfile, std::string certfile, std::string passphrase)
    {
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
                MS_INFO("Listening https port %s:%d success", ip.c_str(), port);
            } else {
                MS_ERROR("Listening https port %s:%d failed.", ip.c_str(), port);
            }
        });

        return 0;
    }

    NetConnection* HttpServer::NewConnection(void *handler)
    {
        HttpConnection *c = nullptr;

        if (this->freeConnections) {
            c = this->freeConnections;
            this->freeConnections = c->next;
        } else {
            c = new HttpConnection(handler);
        }

        c->SetServer(this);

        return c;
    }

    void HttpServer::DeleteConnection(NetConnection *nc)
    {
        HttpConnection *c = (HttpConnection *) nc;

        c->SetContext(nullptr);
        c->SetHandler(nullptr);

        c->next = this->freeConnections;
        this->freeConnections = c;
    }

    int HttpServer::OnPost(void *handle, uWS::HttpRequest *req, std::string_view chunk, bool isEnd, bool ssl)
    {
        HttpConnection *nc = (HttpConnection *) this->FetchConnection(handle);

        if (nc == nullptr) {
            MS_ERROR("Fetch nc failed.");
            return -1;
        }

        nc->SetSSL();

        nc->AppendReceiveBuffer(chunk);

        if (isEnd) {
            if (this->listener) {
                this->listener->OnNetDataReceived(nc);
            }
        }

        return 0;
    }

    void HttpServer::OnDisconnect(void *handle)
    {
        auto it = this->ncMap.find(handle);
        if (it != this->ncMap.end() && this->listener) {
            this->listener->OnNetDisconnected(it->second);
            this->listener = nullptr;
        }

        this->RemoveConnection(handle);
    }

}
