#include "HttpServer.hpp"
#include "Logger.hpp"

#define MS_CLASS "pingos::HttpServer"

namespace pingos {

    void HttpServer::HttpConnection::SetServer(HttpServer *server)
    {
        this->server = server;
    }

    void HttpServer::HttpConnection::SetSSL(bool ssl)
    {
        this->ssl = ssl;
    }

    int HttpServer::HttpConnection::Send(std::string &data)
    {
        if (ssl) {
            auto c = (uWS::HttpResponse<true>*) this->GetHandle();
            if(c != nullptr) {
                c->writeHeader("Access-Control-Allow-Origin", "*");
                c->end(data);
            }
        } else {
            auto c = (uWS::HttpResponse<false>*) this->GetHandle();
            if(c != nullptr) {
                c->writeHeader("Access-Control-Allow-Origin", "*");
                c->end(data);  
            }
        }
        if (this->server) {
            this->server->RemoveConnection(this->GetHandle());
        }

        return 0;
    }

    HttpServer::HttpConnection::HttpConnection(bool ssl)
    {
        ssl = ssl;
    }

    NetConnection* HttpServer::NewConnection(bool ssl)
    {
        HttpConnection *c;
        if (freeConnections) {
            c = freeConnections;
            c->SetSSL(ssl);
            freeConnections = freeConnections->next;
        } else {
            c = new HttpConnection(ssl);
        }

        c->SetServer(this);

        return c;
    }

    void HttpServer::DeleteConnection(NetConnection* c)
    {
        HttpConnection *hc = (HttpConnection*)c;
        hc->next = freeConnections;
        this->freeConnections = hc;

        hc->ClearRecvBuffer();
        hc->SetContext(nullptr);
        hc->SetHandle(nullptr);
    }

    HttpServer::HttpServer()
    {
    
    }

    HttpServer::~HttpServer()
    {
        
    }

    int HttpServer::Start(std::string ip, uint16_t port, std::string pattern)
    {
        if(app == nullptr) {
            app = new uWS::TemplatedApp<false>(uWS::App().post(pattern, [this](auto *res, auto *req) {
                res->onAborted([this, res]() {
                auto c = this->FindConnection((void*)res);
                if(c != nullptr) {
                    if(listener) {
                        listener->OnNetDisconnected(c);
                    }
                }
                RemoveConnection(res);
            });

            res->onData([this, res](std::string_view chunk, bool isEnd) {

                auto c = this->FetchConnection((void*)res, false);
                std::string msg(chunk);
                c->AppendRecvBuffer(msg);
                if (isEnd) {
                    if(c != nullptr && listener!= nullptr) {
                        
                        MS_DEBUG("HttpServer Post message %s", c->GetRecvBuffer().c_str());
                        listener->OnNetDataReceived(c);
                    }
                    c->ClearRecvBuffer();
                };
            });
        }).listen(port, [port](us_listen_socket_t *listensock) {
            if (listensock) {
                MS_INFO("HttpServer Listener Post %d success", port);
            } else {
                MS_ERROR("HttpServer Listener Post %d failed", port);
            }
        })
        );
        }

        return 0;
    }

    int HttpServer::Start(std::string ip, uint16_t port, std::string pattern,
        std::string keyfile, std::string certfile, std::string passphrase)
    {

        if(sslApp == nullptr) {
            sslApp = new uWS::TemplatedApp<true>(uWS::SSLApp({
                .key_file_name = keyfile.c_str(),
                .cert_file_name = certfile.c_str(),
                .passphrase = passphrase.c_str()}));
            
            sslApp->post(pattern, [this](auto *res, auto *req) {
                res->onAborted([this, res]() {
                    auto c = this->FindConnection((void*)res);
                    if(c != nullptr) {
                        if(listener) {
                            listener->OnNetDisconnected(c);
                        }
                    }
                });

                res->onData([this, res](std::string_view chunk, bool isEnd) {
                    auto c = this->FetchConnection((void*)res, true); 
                    std::string msg(chunk);
                    c->AppendRecvBuffer(msg);
                    if (isEnd) {
                        if(c != nullptr && listener != nullptr) {
                            MS_DEBUG("HttpServer Post message %s", c->GetRecvBuffer().c_str());
                            listener->OnNetDataReceived(c); 
                        }
                        c->ClearRecvBuffer();
                    };
                });
            });

            sslApp->listen(port, [port](us_listen_socket_t *listensock) {
                if (listensock) {
                    MS_INFO("HttpServer Listener Post %d success", port);
                } else {
                    MS_ERROR("HttpServer Listener Post %d failed", port);
                }
            });
        }

        return 0;
    }

}
