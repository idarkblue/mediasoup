#define PMS_CLASS "pingos::RtspServer"
#define MS_CLASS PMS_CLASS

#include <map>
#include "sdptransform.hpp"
#include "Log.hpp"
#include "MediaSoupErrors.hpp"
#include "Rtsp/RtspServer.hpp"
#include "Configuration.hpp"

namespace pingos {

static std::string OfferMslabel = "pingos-rtsp-mslabel";
static char const* DateLine()
{
    static char buf[200];
    time_t tt = time(NULL);

    strftime(buf, sizeof buf, "%a, %b %d %Y %H:%M:%S GMT", gmtime(&tt));

    return buf;
}

RtspServer::RtspServer(TcpServer *tcpServer, RtcMaster *rtcMaster)
{
}

RtspServer::~RtspServer()
{

}

int RtspServer::SetMaster(RtcMaster *rtcMaster)
{
    this->rtcMaster = rtcMaster;

    return 0;
}

void RtspServer::OnRtcSessionAck(RtcSession *rtcSession, json &jsonObject)
{
    std::string sessionId;
    std::string streamId;
    std::string ack;
    std::string reason;
    int error;

    JSON_READ_VALUE_THROW(jsonObject, "sessionId", std::string, sessionId);
    JSON_READ_VALUE_THROW(jsonObject, "streamId", std::string, streamId);
    JSON_READ_VALUE_THROW(jsonObject, "ack", std::string, ack);
    JSON_READ_VALUE_THROW(jsonObject, "error", int, error);
    JSON_READ_VALUE_THROW(jsonObject, "reason", std::string, reason);

    Context *ctx = (Context*) rtcSession->GetContext();

    if (ack != "plainTransport") {
        return;
    }

    if (ctx->setupRequests.empty()) {
        return;
    }

    auto &request = ctx->setupRequests[0];

    std::string uri = request.header.GetUri();
    std::string strTrackId = RtspHeaderLines::GetSplitValue(uri, '/', "trackID");
    uint16_t trackId = std::stoi(strTrackId);
    auto it = ctx->tracks.find(trackId);

    if (it == ctx->tracks.end()) {
        request.Error(RTSP_REPLY_CODE_BAD_REQUEST);
        PMS_ERROR("StreamId[{}] SessionId[{}] Invalid Rtsp request, trackId[{}] not found.",
            ctx->streamId, ctx->sessionId, trackId);
        return;
    }

    auto &track = it->second;

    uint16_t port, rtcpPort;

    auto jsonDataIt = jsonObject.find("data");
    if (jsonDataIt == jsonObject.end()) {
        PMS_ERROR("StreamId[{}] SessionId[{}] missing data.", ctx->streamId, ctx->sessionId);
        request.Error(RTSP_REPLY_CODE_INTERNAL_SERVER_ERROR);
        return;
    }

    auto jsonTupleIt = jsonDataIt->find("tuple");
    if (jsonTupleIt == jsonDataIt->end()) {
        PMS_ERROR("StreamId[{}] SessionId[{}] missing tuple.", ctx->streamId, ctx->sessionId);
        request.Error(RTSP_REPLY_CODE_INTERNAL_SERVER_ERROR);
        return;
    }
    auto jsonRtcpTupleIt = jsonDataIt->find("rtcpTuple");
    if (jsonRtcpTupleIt == jsonDataIt->end()) {
        PMS_ERROR("StreamId[{}] SessionId[{}] missing rtcpTuple.", ctx->streamId, ctx->sessionId);
        request.Error(RTSP_REPLY_CODE_INTERNAL_SERVER_ERROR);
        return;
    }

    JSON_READ_VALUE_THROW(*jsonTupleIt, "localPort", uint16_t, port);
    JSON_READ_VALUE_THROW(*jsonRtcpTupleIt, "localPort", uint16_t, rtcpPort);

    RtspHeaderLines lines;
    std::string value = request.header.GetHeaderValue("Transport") + std::string(";");
    value += std::string("source=") + request.header.GetHost() + std::string(";");
    value += std::string("server_port=") + std::to_string(port) + std::string("-") + std::to_string(rtcpPort) + std::string(";");
    value += std::string("ssrc=") + std::to_string(track.ssrc);
    lines.SetHeaderValue("Transport", value);
    request.Accept(lines);

    PMS_INFO("SessionId[{}] StreamId[{}] Ack[{}] error[{}] reason[{}] data[{}]",
        sessionId, streamId, ack, error, reason, jsonObject["data"].dump());
}

void RtspServer::OnRtcSessionEvent(RtcSession *rtcSession, json &jsonObject)
{
    return;
}

void RtspServer::OnRtspTcpConnectionClosed(
    pingos::TcpServer* tcpServer, pingos::TcpConnection* connection)
{
    Context *ctx = (Context *)(connection->GetContext());
    if (ctx) {
        ctx->c = nullptr;
        connection->SetContext(nullptr);
        this->DeleteContext(ctx);
    }

    return;
}


int RtspServer::OnRecvOptions(RtspRemoteRequest &request)
{
    std::string options = "OPTIONS, ANNOUNCE, DESCRIBE, SETUP, TEARDOWN, PLAY, RECORD, PAUSE";

    RtspHeaderLines lines;
    lines.SetHeaderValue("Public", options);

    request.Accept(lines);

    return 0;
}

int RtspServer::OnRecvGetParameter(RtspRemoteRequest &request)
{
    auto ctx = this->GetContext(request);

    if (!ctx || ctx->stage != Stage::STREAM) {
        request.Error(RTSP_REPLY_CODE_BAD_REQUEST);
        PMS_ERROR("Invalid Rtsp request.");
        return -1;
    }

    std::string options = "OPTIONS, ANNOUNCE, DESCRIBE, SETUP, TEARDOWN, PLAY, RECORD, PAUSE";

    RtspHeaderLines lines;
    lines.SetHeaderValue("Public", options);

    request.Accept(lines);

    return 0;
}

int RtspServer::OnRecvDescribe(RtspRemoteRequest &request)
{
    // if (!ctx || ctx->stage != Stage::OPTIONS) {
    //     request.Error(RTSP_REPLY_CODE_BAD_REQUEST);
    //     PMS_ERROR("Invalid Rtsp request.");
    //     return -1;
    // }

    std::string uri = request.header.GetUri();
    if (uri.empty()) {
        request.Error(RTSP_REPLY_CODE_NOT_FOUND);
        PMS_ERROR("Invalid rtsp url, {}", request.header.GetUrl());

        return -1;
    }

    std::vector<std::string> strVec;
    RtspHeaderLines::SplitString(uri, '/', strVec);
    std::string streamId = strVec[strVec.size() - 1];

    auto ctx = this->GetContext(request);

    if (!ctx) {
        auto s = this->rtcMaster->CreateSession(streamId, pingos::RtcSession::Role::RTSP_PLAYER);
        ctx = this->CreateContext(request.connection, s);
        if (ctx == nullptr) {
            request.Error(RTSP_REPLY_CODE_BAD_REQUEST);
            PMS_ERROR("StreamId[{}] create ctx failed", ctx->streamId);
            return -1;
        }
    }

    RtcSession *publisher = this->rtcMaster->FindPublisher(streamId);
    if (publisher == nullptr) {
        request.Error(RTSP_REPLY_CODE_NOT_FOUND);
        return -1;
    }

    std::string sdp = this->GenerateSdp(ctx, publisher->GetProducerParameters());
    if (sdp.empty()) {
        PMS_ERROR("StreamId[{}] Invalid sdp", streamId);
        request.Error(RTSP_REPLY_CODE_INTERNAL_SERVER_ERROR);
        return -1;
    }

    std::string date = DateLine();
    RtspHeaderLines lines;
    lines.SetHeaderValue("Cache-Control", "no-cache");
    lines.SetHeaderValue("Last-Modified", date);
    lines.SetHeaderValue("Expires", date);
    lines.SetHeaderValue("Date", date);
    lines.SetHeaderValue("Session", "40;timeout=20");
    lines.SetHeaderValue("Content-Type", "application/sdp");
    lines.SetHeaderValue("Content-Base", request.header.GetUrl());

    request.Accept(lines, sdp);

    ctx->stage = Stage::DESCRIBE;

    return 0;
}

int RtspServer::OnRecvSetup(RtspRemoteRequest &request)
{
    auto ctx = this->GetContext(request);

    if (!ctx || (ctx->stage != Stage::DESCRIBE && ctx->stage != Stage::SETUP)) {
        request.Error(RTSP_REPLY_CODE_BAD_REQUEST);
        PMS_ERROR("StreamId[{}] SessionId[{}] Invalid Rtsp request, describe needed.",
            ctx->streamId, ctx->sessionId);
        return -1;
    }

    std::string uri = request.header.GetUri();
    std::string strTrackId = RtspHeaderLines::GetSplitValue(uri, '/', "trackID");
    uint16_t trackId = std::stoi(strTrackId);
    auto it = ctx->tracks.find(trackId);

    if (it == ctx->tracks.end()) {
        request.Error(RTSP_REPLY_CODE_BAD_REQUEST);
        PMS_ERROR("StreamId[{}] SessionId[{}] Invalid Rtsp request, trackId[{}] not found.",
            ctx->streamId, ctx->sessionId, trackId);
        return -1;
    }

    auto &track = it->second;

    std::string transportLine = request.header.GetHeaderValue("transport");
    if (transportLine.empty()) {
        request.Error(RTSP_REPLY_CODE_BAD_REQUEST);
        PMS_ERROR("StreamId[{}] SessionId[{}] Invalid Rtsp request, missing transport line.",
            ctx->streamId, ctx->sessionId, trackId);
        return -1;
    }
    std::string clientPorts = RtspHeaderLines::GetSplitValue(transportLine, ';', "client_port");

    std::vector<std::string> strVec;
    RtspHeaderLines::SplitString(clientPorts, '-', strVec);
    if (strVec.empty()) {
        request.Error(RTSP_REPLY_CODE_BAD_REQUEST);
        PMS_ERROR("StreamId[{}] SessionId[{}] Invalid Rtsp request, missing port.",
            ctx->streamId, ctx->sessionId);
        return -1;
    }

    if (strVec.size() == 1) {
        track.remotePort = std::stoi(strVec[0]);
        track.remoteRtcpPort = std::stoi(strVec[0]);
    } else {
        track.remotePort = std::stoi(strVec[0]);
        track.remoteRtcpPort = std::stoi(strVec[1]);
    }

    pingos::PlainTransportConstructor plainTransportParameters;
    plainTransportParameters.announcedIp = Configuration::webrtc.announcedIp;
    plainTransportParameters.listenIp = Configuration::webrtc.listenIp;
    plainTransportParameters.comedia = true;
    plainTransportParameters.rtcpMux = false;
    plainTransportParameters.srtpCryptoSuite = false;
    plainTransportParameters.trackId = track.id;

    ctx->s->CreatePlainTransport(plainTransportParameters);

    ctx->setupRequests.push_back(request);

    return 0;
}

int RtspServer::OnRecvPlay(RtspRemoteRequest &request)
{
    RtspHeaderLines lines;
    auto ctx = this->GetContext(request);

    std::string value = std::string("url=");
    for (auto kv : ctx->tracks) {
        auto &track = kv.second;
        value += request.header.GetUrl() + std::string("/trackID=") + std::to_string(track.id) + std::string(";");
    }

    if (value.at(value.length() - 1) == ';') {
        value.substr(0, value.length() - 1);
    }

    lines.SetHeaderValue("RTP-Info", value);
    lines.SetHeaderValue("Session", ctx->sessionId);

    request.Accept(lines);

    auto *publisher = this->rtcMaster->FindPublisher(ctx->streamId);
    if (publisher == nullptr) {
        PMS_ERROR("StreamId[{}] Publisher not found", ctx->streamId);

        MS_THROW_ERROR("Publisher not found");

        return -1;
    }

    std::vector<pingos::ConsumerParameters> consumerParameters;
    auto &parameters = publisher->GetProducerParameters();
    for (auto parameter : parameters) {
        ConsumerParameters consumerParameter;
        parameter.GenerateConsumer(consumerParameter);
        consumerParameters.push_back(consumerParameter);
    }

    ctx->s->SetProducerParameters(*publisher);
    ctx->s->SetConsumerParameters(consumerParameters);
    for (auto it : ctx->tracks) {
        ctx->s->TrackPlay(it.second.kind, it.second.id);
    }

    return 0;
}

int RtspServer::OnRecvTeardown(RtspRemoteRequest &request)
{
    request.Accept();

    auto ctx = this->GetContext(request);
    if (ctx && ctx->s) {
        this->CloseSession(ctx->s);
    }

    return 0;
}

void RtspServer::OnTcpConnectionPacketReceived(pingos::TcpConnection* connection, RtspHeaderLines &headerLines, std::string body)
{
    RtspRequestHeader header(headerLines);

    RtspRemoteRequest request(connection, header, body);

    std::string method = header.GetMethod();

    if (method == "options") {
        this->OnRecvOptions(request);
    } else if (method == "get_parameter") {
        this->OnRecvGetParameter(request);
    } else if (method == "describe") {
        this->OnRecvDescribe(request);
    } else if (method == "setup") {
        this->OnRecvSetup(request);
    } else if (method == "play") {
        this->OnRecvPlay(request);
    } else if (method == "teardown") {
        this->OnRecvTeardown(request);
    } else {
        request.Error(RtspReplyCode::RTSP_REPLY_CODE_OPTION_NOT_SUPPORTED);
        return;
    }
}

RtspServer::Context* RtspServer::GetContext(RtspRemoteRequest &request)
{
    if (request.connection == nullptr) {
        return nullptr;
    }

    return (RtspServer::Context*) request.connection->GetContext();
}

RtspServer::Context* RtspServer::GetContext(RtcSession *rtcSession)
{
    return (RtspServer::Context*) rtcSession->GetContext();
}

RtspServer::Context* RtspServer::CreateContext(TcpConnection *c, RtcSession *s)
{
    Context *ctx = new Context();

    if (c) {
        ctx->c = c;
        c->SetContext(ctx);
    }

    if (s) {
        ctx->s = s;
        s->SetContext(ctx);
        ctx->sessionId = s->GetSessionId();
        ctx->streamId = s->GetStreamId();
        s->AddListener(this);
    }

    return ctx;
}

void RtspServer::DeleteContext(RtspServer::Context *ctx)
{
    if (ctx->c) {
        ctx->c->SetContext(nullptr);
        ctx->c->Close();
    }

    if (ctx->s) {
        ctx->s->SetContext(nullptr);
        ctx->s->Close();
    }

    ctx->c = nullptr;
    ctx->s = nullptr;

    delete ctx;
}

void RtspServer::CloseSession(RtcSession *session)
{
    if (session && session->GetContext()) {
        RtspServer::Context *ctx = (RtspServer::Context*) session->GetContext();
        session->SetContext(nullptr);
        ctx->s = nullptr;
        this->DeleteContext(ctx);
    }

    session->Close();
}

std::string RtspServer::GenerateSdp(RtspServer::Context *ctx, std::vector<pingos::ProducerParameters> &parameters)
{
    json jsonSdp = json::object();
    json jsonMedias = json::array();

    std::string mids = "";
    uint32_t trackId = 0;

    // sdp: media
    for (auto &producer : parameters) {
        if (producer.kind != "audio" && producer.kind != "video") {
            continue;
        }

        jsonMedias.emplace_back(json::object());
        auto &jsonMedia = jsonMedias[jsonMedias.size() - 1];

        int i = 0;

        jsonMedia["control"] = std::string("trackID=") + std::to_string(trackId);

        // sdp: media: ext
        jsonMedia["ext"] = json::array();
        auto jsonExtIt = jsonMedia.find("ext");
        for (auto &ext : producer.rtpParameters.headerExtensions) {
            jsonExtIt->emplace_back(json::value_t::object);
            (*jsonExtIt)[i]["uri"] = ext.uri;
            (*jsonExtIt)[i]["value"] = ext.id;
            i++;
        }

        // sdp: media: fmtp & rtcpFb
        jsonMedia["fmtp"] = json::array();
        jsonMedia["rtcpFb"] = json::array();
        auto jsonFmtpIt = jsonMedia.find("fmtp");
        auto jsonRtcpFbIt = jsonMedia.find("rtcpFb");
        std::string payloads = "";
        i = 0;
        for (auto &codec : producer.rtpParameters.codecs) {
            jsonFmtpIt->emplace_back(json::value_t::object);

            std::string parameters = "";
            json jsonParameters;
            codec.parameters.FillJson(jsonParameters);
            for (auto &jsonItem : jsonParameters.items()) {
                auto &key = jsonItem.key();
                auto &value = jsonItem.value();

                if (value.is_number()) {
                    parameters += key + "=" + std::to_string(value.get<int>()) + ";";
                } else if (value.is_string()) {
                    parameters += key + "=" + value.get<std::string>() + ";";
                }
            }

            if (!parameters.empty() && parameters.at(parameters.size() - 1) == ';') {
                parameters = parameters.substr(0, parameters.size() - 1);
            }

            (*jsonFmtpIt)[i]["config"] = parameters;
            (*jsonFmtpIt)[i]["payload"] = codec.payloadType;
            i++;

            if (codec.rtcpFeedback.size() > 0) {
                int j = 0;
                jsonRtcpFbIt->emplace_back(json::value_t::object);
                for (auto &fb : codec.rtcpFeedback) {
                    (*jsonRtcpFbIt)[j]["payload"] = codec.payloadType;
                    (*jsonRtcpFbIt)[j]["type"] = fb.type;
                    (*jsonRtcpFbIt)[j]["subtype"] = fb.parameter;
                    j++;
                }
            }

            payloads += std::to_string(codec.payloadType) + " ";
        }

        // sdp: media: payloads
        if (!payloads.empty() && payloads.at(payloads.size() - 1) == ' ') {
            payloads = payloads.substr(0, payloads.size() - 1);
        }
        jsonMedia["payloads"] = payloads;

        // sdp: media: rtp
        jsonMedia["rtp"] = json::array();
        auto jsonRtpIt = jsonMedia.find("rtp");
        i = 0;
        for (auto &rtp : producer.rtpParameters.codecs) {
            if (rtp.mimeType.subtype == RTC::RtpCodecMimeType::Subtype::RTX) {
                continue;
            }
            jsonRtpIt->emplace_back(json::value_t::object);
            auto &jsonRtp = (*jsonRtpIt)[i];
            jsonRtp["payload"] = rtp.payloadType;
            jsonRtp["rate"] = rtp.clockRate;
            jsonRtp["codec"] = RTC::RtpCodecMimeType::subtype2String[rtp.mimeType.subtype];
            if (producer.kind == "audio" && rtp.channels > 0) {
                jsonRtp["encoding"] = rtp.channels;
            }
            i++;
        }

        // sdp: media: ssrcs

        if (producer.direction.find("recv") != std::string::npos) {

            json jsonSsrcs = json::array();
            json jsonGroups = json::array();
            std::string ssrcGroup = "";
            for (auto encoding : producer.rtpParameters.encodings) {
                if (encoding.ssrc == 0) {
                    continue;
                }

                Track track;
                track.kind = producer.kind;
                track.id = trackId++;
                track.ssrc = encoding.ssrc;
                ctx->tracks[track.id] = track;

                json jsonSsrc = json::object();
                jsonSsrc["attribute"] = "cname";
                jsonSsrc["id"] = encoding.ssrc;
                jsonSsrc["value"] = producer.rtpParameters.rtcp.cname;
                jsonSsrcs.push_back(jsonSsrc);

                jsonSsrc = json::object();
                jsonSsrc["attribute"] = "msid";
                jsonSsrc["id"] = encoding.ssrc;
                jsonSsrc["value"] = OfferMslabel + " " + OfferMslabel + "-" + producer.kind;
                jsonSsrcs.push_back(jsonSsrc);

                jsonSsrc = json::object();
                jsonSsrc["attribute"] = "mslabel";
                jsonSsrc["id"] = encoding.ssrc;
                jsonSsrc["value"] = OfferMslabel;
                jsonSsrcs.push_back(jsonSsrc);

                jsonSsrc = json::object();
                jsonSsrc["attribute"] = "label";
                jsonSsrc["id"] = encoding.ssrc;
                jsonSsrc["value"] = OfferMslabel + "-" + producer.kind;
                jsonSsrcs.push_back(jsonSsrc);

                if (!encoding.hasRtx) {
                    continue;
                }

                jsonSsrc = json::object();
                jsonSsrc["attribute"] = "cname";
                jsonSsrc["id"] = encoding.rtx.ssrc;
                jsonSsrc["value"] = producer.rtpParameters.rtcp.cname;
                jsonSsrcs.push_back(jsonSsrc);

                jsonSsrc = json::object();
                jsonSsrc["attribute"] = "msid";
                jsonSsrc["id"] = encoding.rtx.ssrc;
                jsonSsrc["value"] = OfferMslabel + " " + OfferMslabel + "-" + producer.kind;
                jsonSsrcs.push_back(jsonSsrc);

                jsonSsrc = json::object();
                jsonSsrc["attribute"] = "mslabel";
                jsonSsrc["id"] = encoding.rtx.ssrc;
                jsonSsrc["value"] = OfferMslabel;
                jsonSsrcs.push_back(jsonSsrc);

                jsonSsrc = json::object();
                jsonSsrc["attribute"] = "label";
                jsonSsrc["id"] = encoding.rtx.ssrc;
                jsonSsrc["value"] = OfferMslabel + "-" + producer.kind;
                jsonSsrcs.push_back(jsonSsrc);

                json jsonGroup;
                jsonGroup["semantics"] = "FID";
                jsonGroup["ssrcs"] = std::to_string(encoding.ssrc) + " " + std::to_string(encoding.rtx.ssrc);
                jsonGroups.push_back(jsonGroup);
            }

            jsonMedia["ssrcs"] = jsonSsrcs;
            if (jsonGroups.size() > 0) {
                jsonMedia["ssrcGroups"] = jsonGroups;
            }
            // sdp: media: direction
            jsonMedia["direction"] = "sendrecv";

        } else {
            // sdp: media: direction
            jsonMedia["direction"] = "recvonly";
        }

        jsonMedia["direction"] = "sendonly";

        // sdp: media: mid
        jsonMedia["mid"] = producer.rtpParameters.mid;
        // sdp: media: port (ignore)
        jsonMedia["port"] = 9;
        // sdp: media: protocol
        jsonMedia["protocol"] = "RTP/AVP";

        jsonMedia["rtcp"]["address"] = "0.0.0.0";
        jsonMedia["rtcp"]["ipVer"] = 4;
        jsonMedia["rtcp"]["netType"] = "IN";
        jsonMedia["rtcp"]["port"] = 9;

        // sdp: media: rtcpMux
        jsonMedia["rtcpMux"] = "rtcp-mux";
        // sdp: media: rtcpRsize
        jsonMedia["rtcpRsize"] = "rtcp-rsize";
        // sdp: media: x-google-flag
//        jsonMedia["xGoogleFlag"] = "conference";
        // sdp: media: type (audio or video)
        jsonMedia["type"] = producer.kind;
        jsonMedia["connection"]["ip"] = "0.0.0.0";
        jsonMedia["connection"]["version"] = 4;

        mids += producer.rtpParameters.mid + " ";
    }

    jsonSdp["media"] = jsonMedias;

    if (!mids.empty() && mids.at(mids.size() - 1) == ' ') {
        mids = mids.substr(0, mids.size() - 1);
    }

    json jsonGroup = json::object();
    jsonGroup["mids"] = mids;
    jsonGroup["type"] = "BUNDLE";

    jsonSdp["groups"].push_back(jsonGroup);

    jsonSdp["msidSemantic"]["semantic"] = "WMS";
    jsonSdp["msidSemantic"]["token"] = "*";
    jsonSdp["name"] = "-";
    jsonSdp["origin"]["address"] = "0.0.0.0";
    jsonSdp["origin"]["ipVer"] = 4;
    jsonSdp["origin"]["netType"] = "IN";
    jsonSdp["origin"]["sessionId"] = "-";
    jsonSdp["origin"]["sessionVersion"] = 2;
    jsonSdp["origin"]["username"] = "-";

    jsonSdp["timing"]["start"] = 0;
    jsonSdp["timing"]["stop"] = 0;
    jsonSdp["connection"]["ip"] = "0.0.0.0";
    jsonSdp["connection"]["version"] = 4;
    jsonSdp["name"] = ctx->streamId;
    jsonSdp["sdplang"] = "en";
    jsonSdp["version"] = 0;

    std::string sdp = sdptransform::write(jsonSdp);

    PMS_DEBUG("SessionId[{}] StreamId[{}] jsonSdp {}", ctx->sessionId, ctx->streamId, jsonSdp.dump());
    PMS_DEBUG("SessionId[{}] StreamId[{}] sdp {}", ctx->sessionId, ctx->streamId, sdp);

    return sdp;
}

}
