#define MS_CLASS "pingos::WebRtcModule"

#include "WebRtcModule.hpp"
#include "core/Configuration.hpp"
#include "core/ConsoleFilter.hpp"
#include "core/Stream.hpp"
#include "MediaSoupErrors.hpp"
#include "utils/JsonHelper.hpp"

namespace pingos
{
    static std::function< int(Session*)> NextPublishFunctional = nullptr;
    static std::function< int(Session*)> NextPlayFunctional = nullptr;
    static std::function< int(Session*)> NextCloseSessionFunctional = nullptr;

    std::map<std::string, WebRtcModule::Request::MethodId> WebRtcModule::Request::string2MethodId =
    {
        { "session.create",  WebRtcModule::Request::MethodId::SESSION_CREATE },
        { "stream.publish", WebRtcModule::Request::MethodId::STREAM_PUBLISH },
        { "stream.play",    WebRtcModule::Request::MethodId::STREAM_PLAY },
        { "stream.mute",    WebRtcModule::Request::MethodId::STREAM_MUTE },
        { "stream.close",    WebRtcModule::Request::MethodId::STREAM_CLOSE },
        { "session.heartbeat", WebRtcModule::Request::MethodId::SESSION_HEARTBEAT }
    };

    WebRtcModule::Request::Request(NetConnection *nc, json &jsonObject): nc(nc)
    {
        JSON_THROW_READ_VALUE(jsonObject, "method", std::string, string, this->method);
        JSON_READ_VALUE_DEFAULT(jsonObject, "session", std::string, this->session, "");
        JSON_THROW_READ_VALUE(jsonObject, "stream", std::string, string, this->stream);

        if (string2MethodId.find(this->method) == string2MethodId.end()) {
            MS_ERROR("Unknown method[%s]", this->method.c_str());
            MS_THROW_ERROR("Unknown method[%s]", this->method.c_str());
        }

        this->methodId = string2MethodId[this->method];

        auto jsonDataIt = jsonObject.find("data");
        if (jsonDataIt != jsonObject.end()) {
            this->jsonData = *jsonDataIt;
        }
    }

    WebRtcModule::Request::~Request()
    {

    }

    int WebRtcModule::Request::Accept()
    {
        if (this->replied) {
            return 0;
        }

        this->replied = true;

        json jsonResponse = json::object();

        jsonResponse["version"] = "1.0";
        jsonResponse["err"] = 0;
        jsonResponse["err_msg"] = "OK";
        jsonResponse["session"] = this->session;
        jsonResponse["stream"] = this->stream;
        jsonResponse["method"] = this->method;
        jsonResponse["data"] = json::object();

        if (this->nc->ReplyString(jsonResponse.dump()) != 0) {
            MS_ERROR("StreamId[%s] rtc reply failed", this->stream.c_str());
            return -1;
        }

        MS_DEBUG("StreamId[%s] rtc reply: %s", this->stream.c_str(), jsonResponse.dump().c_str());

        return 0;
    }

    int WebRtcModule::Request::Accept(json &jsonData)
    {
        if (this->replied) {
            return 0;
        }

        this->replied = true;

        json jsonResponse = json::object();

        jsonResponse["version"] = "1.0";
        jsonResponse["err"] = 0;
        jsonResponse["err_msg"] = "OK";
        jsonResponse["session"] = this->session;
        jsonResponse["stream"] = this->stream;
        jsonResponse["method"] = this->method;
        jsonResponse["data"] = jsonData;

        if (this->nc->ReplyString(jsonResponse.dump()) != 0) {
            MS_ERROR("StreamId[%s] rtc reply failed", this->stream.c_str());
            return -1;
        }

        MS_DEBUG("StreamId[%s] rtc reply: %s", this->stream.c_str(), jsonResponse.dump().c_str());

        return 0;
    }

    int WebRtcModule::Request::Error(const char* reason)
    {
        if (this->replied) {
            return 0;
        }

        this->replied = true;

        json jsonResponse = json::object();

        jsonResponse["version"] = "1.0";
        jsonResponse["err"] = -1;
        jsonResponse["session"] = this->session;
        jsonResponse["stream"] = this->stream;
        jsonResponse["method"] = this->method;

        if (reason) {
            jsonResponse["err_msg"] = reason;
        } else {
            jsonResponse["err_msg"] = "error";
        }
        jsonResponse["data"] = json::object();

        if (this->nc->ReplyString(jsonResponse.dump()) != 0) {
            MS_ERROR("StreamId[%s] rtc reply failed", this->stream.c_str());
            return -1;
        }

        MS_DEBUG("StreamId[%s] rtc reply: %s", this->stream.c_str(), jsonResponse.dump().c_str());

        return 0;
    }

    WebRtcModule* WebRtcModule::instance = nullptr;

    WebRtcModule::WebRtcModule(): Module("WebRtcModule")
    {

    }

    WebRtcModule::~WebRtcModule()
    {

    }

    void WebRtcModule::ClassInit()
    {
        Module::Register(WebRtcModule::Get());
    }

    WebRtcModule* WebRtcModule::Get()
    {
        if (instance == nullptr) {
            instance = new WebRtcModule();
        }

        return instance;
    }

    void WebRtcModule::OnNetConnected(NetConnection *nc)
    {

    }

    void WebRtcModule::OnNetDataReceived(NetConnection *nc)
    {
        std::string data = nc->GetReceiveBuffer();
        Request *r = nullptr;

        try {
            json jsonObject = json::parse(data);
            r = new Request(nc, jsonObject);

            switch (r->methodId) {
                case Request::MethodId::SESSION_CREATE:
                break;

                case Request::MethodId::STREAM_PUBLISH:
                this->PublishRequest(r);
                break;

                case Request::MethodId::STREAM_PLAY:
                this->PlayRequest(r);
                break;

                case Request::MethodId::STREAM_MUTE:
                this->MuteRequest(r);
                break;

                case Request::MethodId::STREAM_CLOSE:
                this->CloseRequest(r);
                break;

                case Request::MethodId::SESSION_HEARTBEAT:
                this->HartbeatRequest(r);
                break;

                default:
                MS_THROW_ERROR("Invalid MethodId: %s", r->method.c_str());
                break;
            }
        } catch (const json::parse_error &error) {
            MS_ERROR("Catch Json Error: %s", error.what());
            r->Error(error.what());
        } catch (const MediaSoupError &error) {
            MS_ERROR("Catch MS Error: %s", error.what());
            r->Error(error.what());
        }
    }

    void WebRtcModule::OnNetDisconnected(NetConnection *nc)
    {

    }

    void WebRtcModule::ModuleInit()
    {
        NextPublishFunctional = PublishFunctional;
        PublishFunctional = WebRtcModule::PublishHandle;

        NextPlayFunctional = PlayFunctional;
        PlayFunctional = WebRtcModule::PlayHandle;

        NextCloseSessionFunctional = CloseSessionFunctional;
        CloseSessionFunctional = WebRtcModule::CloseSessionHandle;

        StartWss();
    }

    int WebRtcModule::PublishHandle(Session *s)
    {
        json jsonResponse;
        json jsonRemoteConn;
        pingos::SdpHelper::WebRtcConnect localConn;
        std::vector<pingos::SdpHelper::WebRtcConnect> localConns;

        auto *ctx = (Context*) s->GetContext(WebRtcModule::Get()->GetName());
        if (!ctx) {
            goto next;
        }

        for (auto &track : *ctx->tracks.get()) {
            SdpHelper::ReverseTrack(track);
        }

        try {
            auto *stream = FetchStream(s->GetStreamId());

            // may throw
            ConsoleFilter::CreateWebRtcTransport(s, ctx->transportId, Configuration::webRtcTransport.jsonData, jsonResponse);

            // my throw
            SdpHelper::ParseWebRtcTransport(jsonResponse, localConn);

            for (auto &track : *ctx->tracks.get()) {
                localConns.push_back(localConn);
                auto &conn = localConns[localConns.size() - 1];
                conn.mid = track.mid;

                json jsonProducer;
                // may throw
                SdpHelper::Transform2ProducerJson(track, jsonProducer);
                // may throw
                ConsoleFilter::CreateProducer(s, ctx->transportId, jsonProducer);
            }

            std::string sdp = "";

            SdpHelper::Transform2Sdp(localConns, *ctx->tracks.get(), sdp);

            json jsonSdp;
            jsonSdp["sdp"] = sdp;

            ctx->r->Accept(jsonSdp);

            stream->JoinSession(s);

        } catch (const MediaSoupError &error) {
            if (ctx->r) {
                ctx->r->Error(error.what());
            }
        }
next:
        if (NextPublishFunctional) {
            return NextPublishFunctional(s);
        }
        return 0;
    }

    int WebRtcModule::PlayHandle(Session *s)
    {
        if (NextPlayFunctional) {
            return NextPlayFunctional(s);
        }
        return 0;
    }

    int WebRtcModule::CloseSessionHandle(Session *s)
    {
        if (NextCloseSessionFunctional) {
            return NextCloseSessionFunctional(s);
        }

        return 0;
    }

    void WebRtcModule::StartWss()
    {
        this->wss = new WssServer(
            uWS::CompressOptions::SHARED_COMPRESSOR,
            Configuration::websocket.maxPayloadLength,
            Configuration::websocket.idleTimeout,
            Configuration::websocket.maxBackpressure
        );

        if (Configuration::websocket.port) {
            if (this->wss->Accept(Configuration::websocket.listenIp,
                Configuration::websocket.port,
                Configuration::websocket.location) != 0)
            {
                MS_THROW_ERROR("Starting ws %s:%d failed",
                    Configuration::websocket.listenIp.c_str(), Configuration::websocket.port);
            }
        }

        if (!Configuration::websocket.certFile.empty() &&
            !Configuration::websocket.keyFile.empty() &&
            Configuration::websocket.sslPort)
        {
            if (this->wss->Accept(Configuration::websocket.listenIp,
                Configuration::websocket.sslPort,
                Configuration::websocket.location,
                Configuration::websocket.keyFile,
                Configuration::websocket.certFile,
                Configuration::websocket.passPhrase) != 0)
            {
                MS_THROW_ERROR("Starting wss %s:%d failed",
                    Configuration::websocket.listenIp.c_str(), Configuration::websocket.sslPort);
            }
        }

        this->wss->SetListener(WebRtcModule::Get());
    }

    void WebRtcModule::PublishRequest(Request *r)
    {
        std::string streamId;
        std::string sdp;
        std::shared_ptr<std::vector<SdpHelper::Track>> tracks(new std::vector<SdpHelper::Track>);
        std::shared_ptr<std::vector<SdpHelper::WebRtcConnect>> conns(new std::vector<SdpHelper::WebRtcConnect>);
        Session *s = nullptr;

        try {
            JSON_THROW_READ_VALUE(r->jsonData, "sdp", std::string, string, sdp);

            // maybe throw
            SdpHelper::ParseTracks(sdp, *tracks.get());
            if (tracks->size() == 0) {
                MS_THROW_ERROR("Stream[%s] Session[%s] has no tracks",
                    r->stream.c_str(), s->GetSessionId().c_str());
            }
            // maybe throw
            SdpHelper::ParseWebRtcConnect(sdp, *conns.get());

            // init session
            s = this->CreateSession(r, true);
            if (s == nullptr) {
                MS_THROW_ERROR("Stream[%s] create session failed", r->stream.c_str());
            }

            auto *ctx = (Context*) s->GetContext(this->GetName());
            ctx->tracks = tracks;
            ctx->remoteConns = conns;

            ctx->r = r;
            r->count++;
            if (PublishFunctional) {
                if (PublishFunctional(s) != 0) {
                    return;
                }
            }
        } catch (const MediaSoupError &error) {
            if (s) {
                MS_ERROR("Stream[%s] Session[%s] error %s, delete session",
                    r->stream.c_str(), s->GetSessionId().c_str(), error.what());
                delete s;
            }

            MS_THROW_ERROR("%s", error.what());
        }

    }

    void WebRtcModule::PlayRequest(Request *r)
    {

    }

    void WebRtcModule::MuteRequest(Request *r)
    {

    }

    void WebRtcModule::CloseRequest(Request *r)
    {

    }

    void WebRtcModule::HartbeatRequest(Request *r)
    {

    }

    Session* WebRtcModule::CreateSession(Request *r, bool publisher)
    {
        static uint64_t idx = 0;

        std::string sessionId = this->GetName() + std::string("-") + r->stream + std::string("-") + std::to_string(idx);

        auto s = new Session(r->stream, r->session, publisher);

        MS_INFO("Stream[%s] create session[%s]", r->stream.c_str(), sessionId.c_str());

        Context *ctx = new Context();
        s->SetContext(this->GetName(), ctx);
        ctx->routerId = s->GetStreamId();
        ctx->transportId = s->GetSessionId();

        return s;
    }
}
