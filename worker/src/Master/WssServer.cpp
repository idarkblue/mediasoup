#include <string>
#include "Master/Log.hpp"
#include "Master/WssServer.hpp"
#include "Master/Configuration.hpp"

#define PMS_CLASS "pingos::WssServer"

namespace pingos {

#define UApp(SSL_) uWS::TemplatedApp<SSL_>
#define UWS(C_, SSL_)  (uWS::WebSocket<SSL_, true>*)(C_)

WssServer::WssServer(uWS::CompressOptions compression,
            int maxPayloadLength,
            int idleTimeout,
            int maxBackpressure,
            NetServer::Listener *listener)
{
    this->listener = listener;
    this->compression = compression;
    this->maxPayloadLength = maxPayloadLength;
    this->idleTimeout = idleTimeout;
    this->maxBackpressure = maxBackpressure;
}

WssServer::~WssServer()
{

}

struct PerSocketData {
    NetConnection *nc;
};

int WssServer::Accept(std::string ip, uint16_t port, std::string location)
{
    this->port = port;
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
            psd->nc = new NetConnection();
            auto nc = psd->nc;
            nc->Reset(false, ws, this);
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
            PMS_INFO("Listening ws port {}:{} success", ip, port);
        } else {
            PMS_ERROR("Listening ws port {}:{} failed", ip, port);
        }
    });

    return 0;
}

int WssServer::Accept(std::string ip, uint16_t port, std::string location, std::string keyfile, std::string certfile, std::string passphrase)
{
    this->sslPort = port;

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
            psd->nc = new NetConnection();
            auto nc = psd->nc;
            nc->Reset(true, ws, this);
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
            PMS_INFO("Listening wss port {}:{} success", ip, port);
        } else {
            PMS_ERROR("Listening wss port {}:{} failed", ip, port);
        }
    });

    return 0;
}

void WssServer::OnDrain(NetConnection *nc)
{

}

void WssServer::OnMessage(NetConnection *nc, std::string_view message, uWS::OpCode opCode)
{
    PMS_DEBUG("WS Connection ptr {}, message {}, opCode {} ...", (void*)nc, message, (int) opCode);

    if (nc == nullptr) {
        PMS_ERROR("Fetch nc failed.");
        return;
    }

    nc->Padding(message);

    if (this->listener) {
        this->listener->OnMessage(nc);
    }
}

void WssServer::OnPing(NetConnection *nc)
{
    PMS_DEBUG("WS Connection ptr {}, ping ...", (void*)nc);
}

void WssServer::OnPong(NetConnection *nc)
{
    PMS_DEBUG("WS Connection ptr {}, pong ...", (void*)nc);
}

void WssServer::OnClose(NetConnection *nc, int code, std::string_view message)
{
    PMS_INFO("WS Connection ptr {}, closing ...", (void*)nc);

    if (this->listener) {
        this->listener->OnDisconnect(nc);
    }

    delete nc;
}

void WssServer::Close()
{
    PMS_INFO("Closing WS Server");

    if (this->listenSocket) {
        us_listen_socket_close(0, this->listenSocket);
        this->listenSocket = nullptr;
    }
}

int WssServer::Disconnect(NetConnection *nc)
{
    PMS_INFO("Disconnectting WS Connection ptr {}", (void*)nc);

    if (nc->IsSsl()) {
        uWS::WebSocket<true, true> *ws = (uWS::WebSocket<true, true> *)nc->GetConnectionHandler();
        ws->close();
    } else {
        uWS::WebSocket<false, true> *ws = (uWS::WebSocket<false, true> *)nc->GetConnectionHandler();
        ws->close();
    }

    return 0;
}

int WssServer::ReplyBinary(NetConnection *nc, const uint8_t *nsPayload, size_t nsPayloadLen)
{
    if (nc == nullptr || nc->GetConnectionHandler() == nullptr) {
        return -1;
    }

    if (nc->IsSsl()) {
        uWS::WebSocket<true, true> *ws = (uWS::WebSocket<true, true> *)nc->GetConnectionHandler();

        std::string content((const char *)nsPayload, nsPayloadLen);
        ws->send(content, uWS::OpCode::BINARY);
    } else {
        uWS::WebSocket<false, true> *ws = (uWS::WebSocket<false, true> *)nc->GetConnectionHandler();

        std::string content((const char *)nsPayload, nsPayloadLen);
        ws->send(content, uWS::OpCode::BINARY);
    }

    return 0;
}

int WssServer::ReplyString(NetConnection *nc, std::string data)
{
    if (nc == nullptr || nc->GetConnectionHandler() == nullptr) {
        return -1;
    }

    if (nc->IsSsl()) {
        uWS::WebSocket<true, true> *ws = (uWS::WebSocket<true, true> *)nc->GetConnectionHandler();

        ws->send(data, uWS::OpCode::TEXT);
    } else {
        uWS::WebSocket<false, true> *ws = (uWS::WebSocket<false, true> *)nc->GetConnectionHandler();

        ws->send(data, uWS::OpCode::TEXT);
    }

    return 0;
}

}
