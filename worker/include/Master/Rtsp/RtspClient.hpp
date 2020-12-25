#pragma once

#include <unordered_map>
#include "NetServer.hpp"
#include "Master.hpp"
#include "Worker.hpp"
#include "RtcMaster.hpp"
#include "RtcSession.hpp"
#include "Rtsp/TcpClient.hpp"
#include "Rtsp/RtspRequest.hpp"

namespace pingos {
class RtspClient;

class RtspClient : public pingos::TcpClient::Listener,
                   public pingos::TcpConnection::Listener,
                   public RtcSession::Listener
{
public:
    enum RtspClientError {
        FINALISHED,
        DICONNECTED,
        RTSP_SERVER_ERROR
    };
    class Listener {
    public:
        virtual ~Listener() = default;
    public:
        virtual void OnRtspClientPlayCompleted(RtspClient *client) = 0;
        virtual void OnRtspClientPublishCompleted(RtspClient *client) = 0;
        virtual void OnRtspClientError(RtspClient *client, RtspClientError errorCode, RtspReplyCode rtspCode = RtspReplyCode::RTSP_REPLY_CODE_INIT) = 0;
    };

public:
    enum Stage {
        INIT,
        OPTIONS,
        GET_PARAMETER,
        DESCRIBE,
        SETUP,
        PLAY,
        TEARDOWN
    };

    struct RtspURL {
        std::string host;
        uint16_t port;
        std::string uri;
        std::map<std::string, std::string> args;
    };

    RtspClient(RtcMaster *rtcMaster, RtcSession *session);
    virtual ~RtspClient();

    std::string GetURL();
    void SetListener(Listener *listener);
    void SetRtcSession(RtcSession *session);
    RtcSession* GetRtcSession();
    int SetMaster(RtcMaster *master);

    int Play(std::string url);
    int Teardown();

protected:
    int ParseURL(std::string url, RtspURL &u);

// Rtsp interface
protected:
    int RequestOptions();
    int RequestGetParameter();
    int RequestDescribe();
    int RequestSetup(uint16_t trackId, uint16_t rtpPort, uint16_t rtcpPort);
    int RequestPlay();
    int RequestTeardown();

protected:
    int OnOptionsAck(RtspReplyHeader &header);
    int OnGetParameterAck(RtspReplyHeader &header);
    int OnDescribeAck(RtspReplyHeader &header, std::string &body);
    int OnSetupAck(RtspReplyHeader &header);
    int OnPlayAck(RtspReplyHeader &header);
    int OnTeardownAck(RtspReplyHeader &header);

// Implement RtcSession::Listener
protected:
    void OnRtcSessionAck(RtcSession *rtcSession, json &jsonObject) override;
    void OnRtcSessionEvent(RtcSession *rtcSession, json &jsonObject) override;
    void OnRtcSessionClosed(RtcSession *rtcSession) override;

// Implement TcpClient::Listener
protected:
    void OnTcpClientConnected(TcpClient *client, pingos::TcpConnection* connection) override;
    void OnTcpClientClosed(TcpClient *client, pingos::TcpConnection* connection) override;

// Implement ::TcpConnection::Listener
private:
    void OnTcpConnectionPacketReceived(pingos::TcpConnection* connection, RtspHeaderLines &headerLines, std::string body) override;

private:
    RtcMaster *rtcMaster { nullptr };
    pingos::TcpClient tcpClient;
    RtspURL rtspURL;
    std::string url { "" };
    std::string rtspSessionId { "" };
    Stage stage { Stage::INIT };
    uint64_t trackIndex { 0 };
    std::map<uint16_t, uint16_t> rtpPortTrackIdMap;
    RtcSession *rtcSession { nullptr };
    std::vector<pingos::ProducerParameters> producerParameters;
    Listener *listener { nullptr };
    bool closed { true };
};

}
