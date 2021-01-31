#pragma once

#include "core/Module.hpp"
#include "network/WssServer.hpp"
#include "sdp/SdpHelper.hpp"

namespace pingos
{
    class WebRtcModule;

    class WebRtcModule : public Module, public NetServer::Listener {
    public:
        class Request {
        public:
            enum class MethodId {
                INVALID_METHOD = 0,
                SESSION_CREATE = 1,
                STREAM_PUBLISH = 2,
                STREAM_PLAY = 3,
                STREAM_MUTE = 4,
                STREAM_CLOSE = 5,
                SESSION_HEARTBEAT = 6
            };
            static std::map<std::string, MethodId> string2MethodId;

        public:
            Request(NetConnection *nc, json &jsonObject);
            virtual ~Request();

            int Accept();
            int Accept(json &jsonData);
            int Error(const char* reason = nullptr);

        public:
            int count { 0 };
            std::string version { "1.0" };
            std::string method { "" };
            std::string session { "" };
            std::string stream { "" };
            json jsonData;
            NetConnection *nc { nullptr };
            bool replied{ false };

            MethodId methodId { MethodId::INVALID_METHOD };
        };

    public:
        struct Context {
            std::string routerId;
            std::string transportId;
            std::shared_ptr<std::vector<SdpHelper::Track>> tracks;
            std::shared_ptr<std::vector<SdpHelper::WebRtcConnect>> remoteConns;
            Request *r { nullptr };
        };
    public:
        WebRtcModule();
        virtual ~WebRtcModule();

    public:
        static void ClassInit();
        static WebRtcModule* Get();

    // pingos::Module
    protected:
        void ModuleInit() override;

    public:
        void OnNetConnected(NetConnection *nc) override;
        void OnNetDataReceived(NetConnection *nc) override;
        void OnNetDisconnected(NetConnection *nc) override;

    private:
        static int PublishHandle(Session *s);
        static int PlayHandle(Session *s);
        static int CloseSessionHandle(Session *s);

    private:
        void StartWss();

    private:
        void PublishRequest(Request *r);
        void PlayRequest(Request *r);
        void MuteRequest(Request *r);
        void CloseRequest(Request *r);
        void HartbeatRequest(Request *r);
        void ConsoleRequest(Request *r);

    private:
        Session* CreateSession(Request *r, bool publisher);

    private:
        static WebRtcModule *instance;

    private:
        WssServer *wss;
    };
}
