#ifndef _WSS_HPP_INCLUDE_
#define _WSS_HPP_INCLUDE_

#include <string>
#include <map>
#include <list>
#include "App.h"
#include "NetServer.hpp"

namespace pingos {

class WssServer : public NetServer {
public:
    WssServer(uWS::CompressOptions compress,
            int maxPayloadLength,
            int idleTimeout,
            int maxBackpressure,
            Listener *listener = nullptr);
    virtual ~WssServer();

protected:
    void OnDrain(NetConnection *nc);
    void OnMessage(NetConnection *nc, std::string_view message, uWS::OpCode opCode);
    void OnPing(NetConnection *nc);
    void OnPong(NetConnection *nc);
    void OnClose(NetConnection *nc, int code, std::string_view message);

private:
    void Close();

public:
    virtual int Accept(std::string ip, uint16_t port, std::string location) override;
    virtual int Accept(std::string ip, uint16_t port, std::string location, std::string keyfile, std::string certfile, std::string passphrase) override;
    virtual int Disconnect(NetConnection *nc) override;
    virtual int ReplyBinary(NetConnection *nc, const uint8_t *nsPayload, size_t nsPayloadLen) override;
    virtual int ReplyString(NetConnection *nc, std::string data) override;

private:
    uWS::App *app { nullptr };
    uWS::SSLApp *sslApp { nullptr };
    us_listen_socket_t *listenSocket { nullptr };
    uWS::CompressOptions compression;
    int maxPayloadLength;
    int idleTimeout;
    int maxBackpressure;
};

}

#endif
