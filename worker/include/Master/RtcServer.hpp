#pragma once

#include <unordered_map>
#include "NetServer.hpp"
#include "Master.hpp"
#include "Worker.hpp"
#include "RtcRequest.hpp"
#include "RtcMaster.hpp"
#include "json.hpp"

using json = nlohmann::json;

namespace pingos {

class RtcServer : public NetServer::Listener, public RtcSession::Listener {
public:
struct Context {
    enum State {
        INVALID,
        SETUP,
        READY,
        DESTROIED,
        CLOSED
    };

    State state { State::INVALID };
    NetConnection *nc { nullptr };
    std::string appId { "" };
    std::string streamId { "" };
    std::string uid { "" };
};

public:
    RtcServer() = default;
    RtcServer(NetServer *netServer, RtcMaster *rtcMaster);
    virtual ~RtcServer();

    int SetMaster(RtcMaster *rtcMaster);

// Implement RtcSession::Listener
protected:
    void OnRtcSessionAck(RtcSession *rtcSession, json &jsonObject) override;
    void OnRtcSessionEvent(RtcSession *rtcSession, json &jsonObject) override;

// Implement NetServer::Listener
protected:
    virtual int OnMessage(NetConnection *connection) override;
    virtual void OnDisconnect(NetConnection *connection) override;

protected:
    int SetupSession(RtcRequest *request);
    int PublishStream(RtcRequest *request);
    int PlayStream(RtcRequest *request);
    int MuteStream(RtcRequest *request);
    int CloseStream(RtcRequest *request);
    int Heartbeat(RtcRequest *request);

protected:
    RtcSession* CreateSession(NetConnection *nc, std::string streamId, RtcSession::Role role, bool attach);
    RtcSession* FindSession(RtcRequest *request);
    void DeleteSession(RtcSession *session);

private:
    RtcMaster *rtcMaster;
};

}
