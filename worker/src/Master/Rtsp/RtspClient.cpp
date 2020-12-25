#define PMS_CLASS "pingos::RtspClient"
#define MS_CLASS PMS_CLASS

#include <map>
#include "sdptransform.hpp"
#include "Log.hpp"
#include "MediaSoupErrors.hpp"
#include "Rtsp/RtspClient.hpp"
#include "Configuration.hpp"
#include "Rtsp/RtspUtils.hpp"

namespace pingos {

RtspClient::RtspClient(RtcMaster *master, RtcSession *session) :
    rtcMaster(master), tcpClient(this, this, 65536), rtcSession(session)
{
}

RtspClient::~RtspClient()
{

}

std::string RtspClient::GetURL()
{
    return this->url;
}

void RtspClient::SetListener(Listener *listener)
{
    this->listener = listener;
}

void RtspClient::SetRtcSession(RtcSession *session)
{
    this->rtcSession = session;
    session->AddListener(this);
}

RtcSession* RtspClient::GetRtcSession()
{
    return this->rtcSession;
}

int RtspClient::SetMaster(RtcMaster *master)
{
    this->rtcMaster = master;

    return 0;
}

int RtspClient::Play(std::string url)
{
    if (this->ParseURL(url, this->rtspURL) != 0) {
        PMS_ERROR("RtspClient[{}] parse url failed.", url);
        return -1;
    }

    if (this->tcpClient.Connect(this->rtspURL.host.c_str(), this->rtspURL.port) != 0) {
        PMS_ERROR("RtspClient[{}] connect failed", url);
        return -1;
    }

    PMS_DEBUG("RtspClient[{}] connecting", url);

    this->url = url;

    return 0;
}

int RtspClient::Teardown()
{
    return this->RequestTeardown();
}

int RtspClient::ParseURL(std::string url, RtspURL &u)
{
    std::string uri = RtspUtils::RearString(url, "://");
    if (uri.empty()) {
        PMS_ERROR("RtspClient[{}] invalid url.", url);
        return -1;
    }

    std::string host = RtspUtils::FrontString(uri, "/");
    if (host.empty()) {
        PMS_ERROR("RtspClient[{}] invalid url, missing host", url);
        return -1;
    }

    uint16_t port = 554;

    std::string sPort = RtspUtils::RearString(host, ":");
    if (!sPort.empty()) {
        host = RtspUtils::FrontString(host, ":");
        port = std::stoi(sPort);
    }

    uri = RtspUtils::RearString(uri, "/");
    uri += std::string("/") + uri;

    std::string args = RtspUtils::RearString(uri, "?");
    while (!args.empty()) {
        std::string arg = RtspUtils::FrontString(args, "&");
        if (arg.empty()) {
            continue;
        }
        std::string key = RtspUtils::FrontString(arg, "=");
        std::string val = RtspUtils::RearString(arg, "=");
        u.args[key] = val;
    }

    u.host = host;
    u.port = port;
    u.uri = RtspUtils::FrontString(uri, "?");

    return 0;
}

void RtspClient::OnRtcSessionAck(RtcSession *rtcSession, json &jsonObject)
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

    if (ack != "plainTransport") {
        return;
    }

    uint16_t port, rtcpPort;

    auto jsonDataIt = jsonObject.find("data");
    if (jsonDataIt == jsonObject.end()) {
        PMS_ERROR("RtspClient[{}] SessionId[{}] missing data.", this->url, rtcSession->GetSessionId());
        return;
    }

    auto jsonTupleIt = jsonDataIt->find("tuple");
    if (jsonTupleIt == jsonDataIt->end()) {
        PMS_ERROR("RtspClient[{}] SessionId[{}] missing tuple.", this->url, rtcSession->GetSessionId());
        return;
    }

    JSON_READ_VALUE_THROW(*jsonTupleIt, "localPort", uint16_t, port);

#if 0
    auto jsonRtcpTupleIt = jsonDataIt->find("rtcpTuple");
    if (jsonRtcpTupleIt == jsonDataIt->end()) {
        PMS_ERROR("RtspClient[{}] SessionId[{}] missing rtcpTuple.", this->url, rtcSession->GetSessionId());
        return;
    }

    JSON_READ_VALUE_THROW(*jsonRtcpTupleIt, "localPort", uint16_t, rtcpPort);
#else
    rtcpPort = port;
#endif

    std::string transportId;

    JSON_READ_VALUE_THROW(*jsonDataIt, "id", std::string, transportId);
    uint16_t trackId = 0;
    auto pos = transportId.find_last_of("-");
    if (pos != std::string::npos) {
        auto strTrackId = transportId.substr(pos + 1);
        if (strTrackId.empty()) {
            PMS_ERROR("RtspClient[{}] SessionId[{}] missing trackId.", this->url, rtcSession->GetSessionId());
            return;
        }

        trackId = std::stoi(strTrackId);
    }

    this->RequestSetup(trackId, port, rtcpPort);

    rtpPortTrackIdMap[port] = trackId;

    PMS_INFO("RtspClient[{}] SessionId[{}] StreamId[{}] Ack[{}] error[{}] reason[{}] data[{}]",
        this->url, sessionId, streamId, ack, error, reason, jsonObject["data"].dump());
}

void RtspClient::OnRtcSessionEvent(RtcSession *rtcSession, json &jsonObject)
{
    return;
}

void RtspClient::OnRtcSessionClosed(RtcSession *rtcSession)
{
    PMS_INFO("SessionId[{}] StreamId[{}] rtc session closed.", rtcSession->GetSessionId(), rtcSession->GetStreamId());
    if (!this->closed) {
        this->tcpClient.Close();
    }

    this->rtcSession = nullptr;

    return;
}

int RtspClient::RequestOptions()
{
    RtspLocalRequest request(this->tcpClient.GetTcpConnection());

    RtspRequestHeader header;
    header.SetMethod("OPTIONS", this->url);

    if (request.Send(header, "") != 0) {
        PMS_ERROR("RtspClient[{}] Request OPTIONS failed", this->url);
        return -1;
    }

    PMS_INFO("RtspClient[{}] Request OPTIONS", this->url);

    this->stage = Stage::OPTIONS;

    return 0;
}

int RtspClient::RequestGetParameter()
{
    RtspLocalRequest request(this->tcpClient.GetTcpConnection());

    RtspRequestHeader header;
    header.SetMethod("GET_PARAMETER", this->url);

    if ( request.Send(header, "") != 0) {
        PMS_ERROR("RtspClient[{}] Request GET_PARAMETER failed", this->url);
        return -1;
    }

    PMS_DEBUG("RtspClient[{}] Request GET_PARAMETER", this->url);

    this->stage = Stage::GET_PARAMETER;

    return 0;
}

int RtspClient::RequestDescribe()
{
    RtspLocalRequest request(this->tcpClient.GetTcpConnection());

    RtspRequestHeader header;
    header.SetMethod("DESCRIBE", this->url);

    if (request.Send(header, "") != 0) {
        PMS_ERROR("RtspClient[{}] Request DESCRIBE failed", this->url);
        return -1;
    }

    PMS_INFO("RtspClient[{}] Request DESCRIBE", this->url);

    this->stage = Stage::DESCRIBE;

    return 0;
}

int RtspClient::RequestSetup(uint16_t trackId, uint16_t rtpPort, uint16_t rtcpPort)
{
    RtspLocalRequest request(this->tcpClient.GetTcpConnection());

    RtspRequestHeader header;
    std::string setupLine = this->url;

    auto pos = this->url.find("?");
    if (pos != std::string::npos) {
        setupLine = this->url.substr(0, pos);
    }

    setupLine += std::string("/trackID=") + std::to_string(trackId);
    header.SetMethod("SETUP", setupLine);

    //Transport: RTP/AVP/UDP;unicast;client_port=11460-11461
    std::string transportLine = std::string("RTP/AVP/UDP;unicast;client_port=") + std::to_string(rtpPort) + std::string("-") + std::to_string(rtcpPort);
    header.SetHeaderValue("Transport", transportLine);
    header.SetHeaderValue("Session", this->rtspSessionId);

    if (request.Send(header, "") != 0) {
        PMS_ERROR("RtspClient[{}] Request SETUP failed", this->url);
        return -1;
    }

    PMS_INFO("RtspClient[{}] Request SETUP", this->url);

    this->stage = Stage::SETUP;

    return 0;
}

int RtspClient::RequestPlay()
{
    RtspLocalRequest request(this->tcpClient.GetTcpConnection());

    RtspRequestHeader header;
    header.SetMethod("PLAY", this->url);
    header.SetHeaderValue("Session", this->rtspSessionId);

    if (request.Send(header, "") != 0) {
        PMS_ERROR("RtspClient[{}] Request PLAY failed", this->url);
        return -1;
    }

    PMS_INFO("RtspClient[{}] Request PLAY", this->url);

    this->stage = Stage::PLAY;

    return 0;
}

int RtspClient::RequestTeardown()
{
    RtspLocalRequest request(this->tcpClient.GetTcpConnection());

    RtspRequestHeader header;
    header.SetMethod("TEARDOWN", this->url);
    header.SetHeaderValue("Session", this->rtspSessionId);

    if (request.Send(header, "") != 0) {
        PMS_ERROR("RtspClient[{}] Request TEARDOWN failed", this->url);
        return -1;
    }

    PMS_INFO("RtspClient[{}] Request TEARDOWN", this->url);

    this->stage = Stage::TEARDOWN;

    return 0;
}

int RtspClient::OnOptionsAck(RtspReplyHeader &header)
{
    PMS_INFO("RtspClient[{}] options ack", this->url);

    return 0;
}

int RtspClient::OnGetParameterAck(RtspReplyHeader &header)
{
    PMS_DEBUG("RtspClient[{}] get parameter ack", this->url);

    return 0;
}

int RtspClient::OnDescribeAck(RtspReplyHeader &header, std::string &body)
{
    PMS_INFO("RtspClient[{}] describe ack", this->url);

    if (body.empty()) {
        PMS_ERROR("RtspClient[{}] sdp is empty", this->url);
        return -1;
    }

    SdpInfo si(body);

    if (si.TransformSdp(this->producerParameters) != 0) {
        PMS_ERROR("RtspClient[{}] Transform sdp error", this->url);
        return -1;
    }

    for (auto &producer : this->producerParameters) {
        pingos::PlainTransportConstructor plainTransportParameters;
        plainTransportParameters.announcedIp = Configuration::webrtc.announcedIp;
        plainTransportParameters.listenIp = Configuration::webrtc.listenIp;
        plainTransportParameters.comedia = false;
        plainTransportParameters.rtcpMux = true;
        plainTransportParameters.srtpCryptoSuite = false;
        plainTransportParameters.trackId = producer.trackId;

        this->rtcSession->CreatePlainTransport(plainTransportParameters);
    }

    this->rtcSession->SetProducerParameters(this->producerParameters);

    auto sessionLine = header.GetHeaderValue("session");
    this->rtspSessionId = RtspUtils::FrontString(sessionLine, ";");

    return 0;
}

int RtspClient::OnSetupAck(RtspReplyHeader &header)
{
    std::string transportLine = header.GetHeaderValue("transport");
    std::string strClientPorts = RtspHeaderLines::GetSplitValue(transportLine, ';', "client_port");
    std::string strServerPorts = RtspHeaderLines::GetSplitValue(transportLine, ';', "server_port");
    std::string source = RtspHeaderLines::GetSplitValue(transportLine, ';', "source");

    if (strClientPorts.empty()) {
        PMS_ERROR("RtspClient[{}] missing client_port", this->url);
        return -1;
    }

    if (strServerPorts.empty()) {
        PMS_ERROR("RtspClient[{}] missing server_port", this->url);
        return -1;
    }

    std::vector<std::string> kvs;
    RtspHeaderLines::SplitString(strClientPorts, "-", kvs);
    if (kvs.size() != 2 || kvs[0].empty() || kvs[1].empty()) {
        PMS_ERROR("RtspClient[{}] missing client_port, invalid format [{}]",
            this->url, transportLine);
        return -1;
    }

    uint16_t clientRtpPort = std::stoi(kvs[0]);
//    uint16_t clientRtcpPort = std::stoi(kvs[1]);

    auto it = this->rtpPortTrackIdMap.find(clientRtpPort);
    if (it == this->rtpPortTrackIdMap.end()) {
        PMS_ERROR("RtspClient[{}] no track for port {}", this->url, clientRtpPort);
        return -1;
    }

    uint16_t trackId = it->second;

    kvs.clear();
    RtspHeaderLines::SplitString(strServerPorts, "-", kvs);
    if (kvs.size() != 2 || kvs[0].empty() || kvs[1].empty()) {
        PMS_ERROR("RtspClient[{}] missing server_port, invalid format [{}]",
            this->url, transportLine);
        return -1;
    }

    uint16_t serverRtpPort = std::stoi(kvs[0]);
    uint16_t serverRtcpPort = std::stoi(kvs[1]);

    for (auto &producer : this->producerParameters) {
        if (producer.trackId == trackId) {
            this->rtcSession->TrackPublish(producer.kind, producer.trackId);
        }
    }

    if (this->rtcSession->ConnectPlainTransport(source, serverRtpPort, serverRtcpPort, trackId) != 0) {
        PMS_ERROR("RtspClient[{}] connect plain transport failed", this->url);
        return -1;
    }

    return 0;
}

int RtspClient::OnPlayAck(RtspReplyHeader &header)
{
    if (this->listener) {
        this->listener->OnRtspClientPlayCompleted(this);
    }

    return 0;
}

int RtspClient::OnTeardownAck(RtspReplyHeader &header)
{
    this->tcpClient.Close();

    return 0;
}

void RtspClient::OnTcpConnectionPacketReceived(pingos::TcpConnection* connection, RtspHeaderLines &headerLines, std::string body)
{
    RtspReplyHeader header(headerLines);

    try {
        int result = header.GetResultCode();

        if (result != RtspReplyCode::RTSP_REPLY_CODE_OK) {
            MS_THROW_ERROR("ack code %d", result);
        }

        switch (this->stage) {
            case Stage::OPTIONS:
                if (this->OnOptionsAck(header) != 0) {
                    break;
                }

                if (this->RequestDescribe() != 0) {
                    break;
                }

            break;

            case Stage::DESCRIBE:
                if (this->OnDescribeAck(header, body) != 0) {
                    break;
                }

            break;

            case Stage::SETUP:
                if (this->OnSetupAck(header) != 0) {
                    break;
                }

                trackIndex++;
                if (trackIndex == this->producerParameters.size()) {
                    if (this->RequestPlay() != 0) {
                        break;
                    }
                }
            break;

            case Stage::PLAY:
                this->OnPlayAck(header);
            break;

            case Stage::TEARDOWN:
                this->OnTeardownAck(header);
            break;

            default:
                PMS_ERROR("RtspClient[{}] invalid stage", this->url);
            break;
        }
    } catch (MediaSoupError &error) {
        PMS_ERROR("RtspClient[{}] request error, reason {}", this->url, error.what());
        connection->Close();
    }
}

void RtspClient::OnTcpClientConnected(TcpClient *client, pingos::TcpConnection* connection)
{
    if (this->RequestOptions() != 0) {
        PMS_ERROR("RtspClient[{}] Request options failed.", this->url);
        client->Close();
        return;
    } else {
        this->closed = false;
        PMS_INFO("RtspClient[{}] Request options success.", this->url);
    }
}

void RtspClient::OnTcpClientClosed(TcpClient *client, pingos::TcpConnection* connection)
{
    PMS_INFO("RtspClient[{}] connection closed.", this->url);

    if (this->listener) {
        this->listener->OnRtspClientError(this, DICONNECTED);
    }

    if (this->rtcSession) {
        this->rtcMaster->DeleteSession(this->rtcSession->GetStreamId(), this->rtcSession->GetSessionId());
        this->rtcSession = nullptr;
    }

    this->closed = true;
}

}
