#pragma once

#include <unordered_map>
#include "NetServer.hpp"
#include "Master.hpp"
#include "Worker.hpp"
#include "RtcMaster.hpp"
#include "Rtsp/TcpConnection.hpp"
#include "Rtsp/TcpServer.hpp"
#include "Rtsp/RtspRequest.hpp"

namespace pingos {
class RtspServer : public TcpServer::Listener,
                   public TcpConnection::Listener,
                   public RtcSession::Listener
{
public:
    enum Stage {
        INIT,
        OPTIONS,
        DESCRIBE,
        SETUP,
        STREAM
    };

    struct Track {
        std::string kind;
        uint32_t ssrc { 0 };
        uint16_t id { 0 };
        uint16_t remotePort { 0 };
        uint16_t remoteRtcpPort { 0 };
        uint16_t localPort { 0 };
        uint16_t localRtcpPort { 0 };
    };

    struct Context {
        TcpConnection *c { nullptr };
        RtcSession *s { nullptr };
        std::string sessionId { "" };
        std::string streamId { "" };
        std::map<uint16_t, Track> tracks;
        Stage stage { Stage::INIT };
        std::vector<RtspRemoteRequest> setupRequests;
    };

    RtspServer() = default;
    RtspServer(TcpServer *tcpServer, RtcMaster *rtcMaster);
    virtual ~RtspServer();

    int SetMaster(RtcMaster *rtcMaster);

// Rtsp interface
protected:
    int OnRecvOptions(RtspRemoteRequest &request);
    int OnRecvGetParameter(RtspRemoteRequest &request);
    int OnRecvDescribe(RtspRemoteRequest &request);
    int OnRecvSetup(RtspRemoteRequest &request);
    int OnRecvPlay(RtspRemoteRequest &request);
    int OnRecvTeardown(RtspRemoteRequest &request);

// Implement RtcSession::Listener
protected:
    void OnRtcSessionAck(RtcSession *rtcSession, json &jsonObject) override;
    void OnRtcSessionEvent(RtcSession *rtcSession, json &jsonObject) override;

// Implement TcpServer::Listener
protected:
    void OnRtspTcpConnectionClosed(
        TcpServer* tcpServer, TcpConnection* connection) override;

// Implement TcpConnection::Listener
private:
    void OnTcpConnectionPacketReceived(TcpConnection* connection, RtspHeaderLines &headerLines, std::string body) override;

protected:
    RtspServer::Context* GetContext(RtspRemoteRequest &request);
    RtspServer::Context* GetContext(RtcSession *rtcSession);
    RtspServer::Context* CreateContext(TcpConnection *c, RtcSession *s);
    void DeleteContext(RtspServer::Context *ctx);
    void CloseSession(RtcSession *session);

protected:
    std::string GenerateSdp(RtspServer::Context *ctx, std::vector<ProducerParameters> &parameters);

private:
    RtcMaster *rtcMaster;
};

}
