#include "Logger.hpp"
#include "WssServer.hpp"

#define MS_CLASS "pingos::WssServer"

namespace pingos {

    WssConnection::WssConnection(void *handler) : NetConnection(handler)
    {

    }

    WssConnection::~WssConnection()
    {
    }

    void WssConnection::SetSSL()
    {
        this->ssl = true;
    }

    int WssConnection::ReplyBinary(const uint8_t *nsPayload, size_t nsPayloadLen)
    {
        if (this->ssl) {
            uWS::WebSocket<true, true> *ws = (uWS::WebSocket<true, true> *)this->GetHandler();

            std::string content((const char *)nsPayload, nsPayloadLen);
            ws->send(content, uWS::OpCode::BINARY);
        } else {
            uWS::WebSocket<false, true> *ws = (uWS::WebSocket<false, true> *)this->GetHandler();

            std::string content((const char *)nsPayload, nsPayloadLen);
            ws->send(content, uWS::OpCode::BINARY);
        }

        return 0;
    }

    int WssConnection::ReplyString(std::string data)
    {
        if (this->ssl) {
            uWS::WebSocket<true, true> *ws = (uWS::WebSocket<true, true> *)this->GetHandler();

            ws->send(data, uWS::OpCode::TEXT);
        } else {
            uWS::WebSocket<false, true> *ws = (uWS::WebSocket<false, true> *)this->GetHandler();

            ws->send(data, uWS::OpCode::TEXT);
        }

        return 0;
    }

    WssServer::WssServer(uWS::CompressOptions compression, int maxPayloadLength,
                int idleTimeout, int maxBackpressure):
                compression(compression),
                maxPayloadLength(maxPayloadLength),
                idleTimeout(idleTimeout),
                maxBackpressure(maxBackpressure)
    {
    }

    WssServer::~WssServer()
    {

    }

    struct PerSocketData {
        WssConnection *nc;
    };

    int WssServer::Accept(std::string ip, uint16_t port, std::string location)
    {
        this->app = new uWS::App();

        this->app->ws<PerSocketData>(location, {
            /* Settings */
            .compression = this->compression,
            .maxPayloadLength = this->maxPayloadLength,
            .idleTimeout = this->idleTimeout,
            .maxBackpressure = this->maxBackpressure,
            /* Handlers */
            .upgrade = nullptr,
            .open = [this](auto *ws) {
                auto psd = (PerSocketData *) ws->getUserData();
                psd->nc = (WssConnection*) this->FetchConnection(ws);
            },
            .message = [this](auto *ws, std::string_view message, uWS::OpCode opCode) {
                auto psd = (PerSocketData *) ws->getUserData();
                auto nc = psd->nc;
                this->OnMessage(nc, message, opCode);
            },
            .drain = [this](auto *ws) {
                auto psd = (PerSocketData *) ws->getUserData();
                auto nc = psd->nc;
                this->OnDrain(nc);
            },
            .ping = [this](auto *ws) {
                auto psd = (PerSocketData *) ws->getUserData();
                auto nc = psd->nc;
                this->OnPing(nc);
            },
            .pong = [this](auto *ws) {
                auto psd = (PerSocketData *) ws->getUserData();
                auto nc = psd->nc;
                this->OnPong(nc);
            },
            .close = [this](auto *ws, int code, std::string_view message) {
                auto psd = (PerSocketData *) ws->getUserData();
                auto nc = psd->nc;
                this->OnClose(nc, code, message);
            }
        }).listen(ip, port, LIBUS_LISTEN_EXCLUSIVE_PORT, [this, ip, port](auto *token) {
            this->listenSocket = token;
            if (token) {
                MS_INFO("Listening ws port %s:%d success", ip.c_str(), port);
            } else {
                MS_ERROR("Listening ws port %s:%d failed", ip.c_str(), port);
            }
        });

        return 0;
    }

    int WssServer::Accept(std::string ip, uint16_t port, std::string location, std::string keyfile, std::string certfile, std::string passphrase)
    {
        this->sslApp = new uWS::SSLApp({
            .key_file_name = keyfile.c_str(),
            .cert_file_name = certfile.c_str(),
            .passphrase = passphrase.c_str(),
            .dh_params_file_name = nullptr,
            .ca_file_name = nullptr,
            .ssl_prefer_low_memory_usage = 0
        });

        this->sslApp->ws<PerSocketData>(location, {
            /* Settings */
            .compression = this->compression,
            .maxPayloadLength = this->maxPayloadLength,
            .idleTimeout = this->idleTimeout,
            .maxBackpressure = this->maxBackpressure,
            /* Handlers */
            .upgrade = nullptr,
            .open = [this](auto *ws) {
                auto psd = (PerSocketData *) ws->getUserData();
                psd->nc = (WssConnection*) this->FetchConnection(ws);
                psd->nc->SetSSL();
            },
            .message = [this](auto *ws, std::string_view message, uWS::OpCode opCode) {
                auto psd = (PerSocketData *) ws->getUserData();
                auto nc = psd->nc;
                this->OnMessage(nc, message, opCode);
            },
            .drain = [this](auto *ws) {
                auto psd = (PerSocketData *) ws->getUserData();
                auto nc = psd->nc;
                this->OnDrain(nc);
            },
            .ping = [this](auto *ws) {
                auto psd = (PerSocketData *) ws->getUserData();
                auto nc = psd->nc;
                this->OnPing(nc);
            },
            .pong = [this](auto *ws) {
                auto psd = (PerSocketData *) ws->getUserData();
                auto nc = psd->nc;
                this->OnPong(nc);
            },
            .close = [this](auto *ws, int code, std::string_view message) {
                auto psd = (PerSocketData *) ws->getUserData();
                auto nc = psd->nc;
                this->OnClose(nc, code, message);
            }
        }).listen(ip, port, LIBUS_LISTEN_EXCLUSIVE_PORT, [this, ip, port](auto *token) {
            this->listenSocket = token;
            if (token) {
                MS_INFO("Listening wss port %s:%d success", ip.c_str(), port);
            } else {
                MS_ERROR("Listening wss port %s:%d failed", ip.c_str(), port);
            }
        });

        return 0;
    }

    NetConnection* WssServer::NewConnection(void *handler)
    {
        return new WssConnection(handler);
    }

    void WssServer::DeleteConnection(NetConnection *nc)
    {
        delete (WssConnection*) nc;
    }

    void WssServer::OnDrain(NetConnection *nc)
    {

    }

    void WssServer::OnMessage(NetConnection *nc, std::string_view message, uWS::OpCode opCode)
    {
        MS_DEBUG("WS Connection ptr %p, message %.*s, opCode %d ...", (void*)nc, message.length(), message.data(), (int) opCode);

        if (nc == nullptr) {
            MS_ERROR("Fetch nc failed.");
            return;
        }

        nc->AppendReceiveBuffer(message);

        if (this->listener) {
            this->listener->OnNetDataReceived(nc);
        }
    }

    void WssServer::OnPing(NetConnection *nc)
    {
        MS_DEBUG("WS Connection ptr %p, ping ...", (void*)nc);
    }

    void WssServer::OnPong(NetConnection *nc)
    {
        MS_DEBUG("WS Connection ptr %p, pong ...", (void*)nc);
    }

    void WssServer::OnClose(NetConnection *nc, int code, std::string_view message)
    {
        MS_INFO("WS Connection ptr %p, closing ...", (void*)nc);

        if (this->listener) {
            this->listener->OnNetDisconnected(nc);
        }

        this->RemoveConnection(nc->GetHandler());
    }

    void WssServer::Close()
    {
        MS_INFO("Closing WS Server");

        if (this->listenSocket) {
            us_listen_socket_close(0, this->listenSocket);
            this->listenSocket = nullptr;
        }
    }
}
