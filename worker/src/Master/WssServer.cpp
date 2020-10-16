#include <string>
#include "Master/Log.hpp"
#include "Master/WssServer.hpp"

#define PMS_CLASS "pingos::WssServer"

namespace pingos {

#define UApp(SSL_) uWS::TemplatedApp<SSL_>
#define UWS(C_, SSL_)  (uWS::WebSocket<SSL_, true>*)(C_)

WssServer::WssServer(NetServer::Listener *listener)
{
    m_listener = listener;
}

WssServer::~WssServer()
{

}

int WssServer::Accept(uint16_t port)
{
    m_port = port;
    m_app = new uWS::App();

    m_app->ws<NetConnection>("/*", {
        /* Settings */
        .compression = uWS::SHARED_COMPRESSOR,
        .maxPayloadLength = 16 * 1024 * 1024,
        .idleTimeout = 100,
        .maxBackpressure = 1 * 1024 * 1204,
        /* Handlers */
        .open = [this](auto *ws, auto *req) {
            this->OnOpen(ws, req, false);
        },
        .message = [this](auto *ws, std::string_view message, uWS::OpCode opCode) {
            this->OnMessage(ws, message, opCode, false);
        },
        .drain = [this](auto *ws) {
            this->OnDrain(ws, false);
        },
        .ping = [this](auto *ws) {
            this->OnPing(ws, false);
        },
        .pong = [this](auto *ws) {
            this->OnPong(ws, false);
        },
        .close = [this](auto *ws, int code, std::string_view message) {
            this->OnClose(ws, code, message, false);
        }
    }).listen(port, [this, port](auto *token) {
        m_listenSocket = token;
        if (token) {
            PMS_INFO("WebSocket is listening on port {}", port);
        } else {
            PMS_ERROR("WebSocket is listening on port {} failed", port);
        }
    });

    return 0;
}

int WssServer::Accept(uint16_t port, std::string keyfile, std::string certfile, std::string passphrase)
{
    m_sslPort = port;

    m_sslApp = new uWS::SSLApp({
        .key_file_name = keyfile.c_str(),
        .cert_file_name = certfile.c_str(),
        .passphrase = passphrase.c_str()
    });

    m_sslApp->ws<NetConnection>("/*", {
        /* Settings */
        .compression = uWS::SHARED_COMPRESSOR,
        .maxPayloadLength = 16 * 1024 * 1024,
        .idleTimeout = 100,
        .maxBackpressure = 1 * 1024 * 1204,
        /* Handlers */
        .open = [this](auto *ws, auto *req) {
            this->OnOpen(ws, req, true);
        },
        .message = [this](auto *ws, std::string_view message, uWS::OpCode opCode) {
            this->OnMessage(ws, message, opCode, true);
        },
        .drain = [this](auto *ws) {
            this->OnDrain(ws, true);
        },
        .ping = [this](auto *ws) {
            this->OnPing(ws, true);
        },
        .pong = [this](auto *ws) {
            this->OnPong(ws, true);
        },
        .close = [this](auto *ws, int code, std::string_view message) {
            this->OnClose(ws, code, message, true);
        }
    }).listen(port, [this, port](auto *token) {
        m_listenSocket = token;
        if (token) {
            PMS_INFO("WebSocket is listening on port {}", port);
        } else {
            PMS_ERROR("WebSocket is listening on port {} failed", port);
        }
    });

    return 0;
}

void WssServer::OnListen(us_listen_socket_t *listenSocket)
{

}

void WssServer::OnOpen(void *c, uWS::HttpRequest *req, bool ssl)
{
    NetConnection *nc;

    PMS_INFO("WS connection ptr {}, success", c);

    if (ssl) {
        uWS::WebSocket<true, true> *ws = (uWS::WebSocket<true, true> *)c;
        nc = (NetConnection *) ws->getUserData();
    } else {
        uWS::WebSocket<false, true> *ws = (uWS::WebSocket<false, true> *)c;
        nc = (NetConnection *) ws->getUserData();
    }

    nc->Reset(ssl, c, this);

    this->AddConnection(c, nc);
}

void WssServer::OnDrain(void *c, bool ssl)
{

}

void WssServer::OnMessage(void *c, std::string_view message, uWS::OpCode opCode, bool ssl)
{
    PMS_DEBUG("WS Connection ptr {}, message {}, opCode {} ...", c, message, (int) opCode);
    NetConnection *nc = this->FetchConnection(c, ssl);

    if (nc == nullptr) {
        PMS_ERROR("Fetch nc failed.");
        return;
    }

    nc->Padding(message);

    if (m_listener) {
        m_listener->OnMessage(nc);
    }
}

void WssServer::OnPing(void *c, bool ssl)
{
    PMS_DEBUG("WS Connection ptr {}, ping ...", c);
}

void WssServer::OnPong(void *c, bool ssl)
{
    PMS_DEBUG("WS Connection ptr {}, pong ...", c);
}

void WssServer::OnClose(void *c, int code, std::string_view message, bool ssl)
{
    PMS_INFO("WS Connection ptr {}, closing ...", c);

    auto it = m_ncMap.find(c);
    if (it != m_ncMap.end() && m_listener) {
        m_listener->OnDisconnect(it->second);
    }

    this->RemoveConnection(c);
}

void WssServer::Close()
{
    PMS_INFO("Closing WS Server");

    if (m_listenSocket) {
        us_listen_socket_close(0, m_listenSocket);
        m_listenSocket = nullptr;
    }
}

int WssServer::Disconnect(NetConnection *nc)
{
    PMS_INFO("Disconnectting WS Connection ptr {}", nc->GetConnectionHandler());

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
        uWS::WebSocket<true, true> *ws = (uWS::WebSocket<true, true> *)nc->GetConnectionHandler();

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
        uWS::WebSocket<true, true> *ws = (uWS::WebSocket<true, true> *)nc->GetConnectionHandler();

        ws->send(data, uWS::OpCode::TEXT);
    }

    return 0;
}

}
