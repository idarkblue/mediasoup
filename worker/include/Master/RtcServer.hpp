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

    int Start(NetServer *netServer, RtcMaster *rtcMaster);

// Implement RtcSession::Listener
protected:
    void OnRtcSessionAck(RtcSession *rtcSession, json &jsonObject) override;
    void OnRtcSessionEvent(RtcSession *rtcSession, json &jsonObject) override;

// Implement NetServer::Listener
public:
    virtual int OnMessage(NetConnection *connection) override;
    virtual void OnDisconnect(NetConnection *connection) override;

protected:
    int PublishStream(RtcRequest *request);
    int PlayStream(RtcRequest *request);
    int MuteStream(RtcRequest *request);
    int CloseStream(RtcRequest *request);

protected:
    RtcWorker* FindWorkerByStreamId(std::string streamId);
    Context* GetContext(NetConnection *nc);
    std::string SpellSessionId();
    RtcSession* CreateRtcSession(RtcRequest *request);
    void DeleteSession(RtcSession *session);

private:
    NetServer *m_netServer;
    RtcMaster *m_rtcMaster;
};

}
