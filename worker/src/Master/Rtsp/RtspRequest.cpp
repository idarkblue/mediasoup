#define PMS_CLASS "pingos::RtspRemoteRequest"
#define MS_CLASS PMS_CLASS

#include <map>
#include "Rtsp/RtspRequest.hpp"
#include "MediaSoupErrors.hpp"

namespace pingos {

static std::map<RtspReplyCode, std::string> RtspReplyCodeString = {
    { RTSP_REPLY_CODE_CONTINUE, "Continue" },
    { RTSP_REPLY_CODE_TIMEOUT, "Connect Timeout" },
    { RTSP_REPLY_CODE_OK, "OK" },
    { RTSP_REPLY_CODE_CREATED, "Created" },
    { RTSP_REPLY_CODE_LOW_SPACE, "Low on Storage Space" },
    { RTSP_REPLY_CODE_MULTIPLE_CHOICES, "Multiple Choices" },
    { RTSP_REPLY_CODE_MOVED_PERMANENTLY, "Moved Permanently" },
    { RTSP_REPLY_CODE_MOVED_TEMP, "Moved Temporarily" },
    { RTSP_REPLY_CODE_SEE_OTHER, "See Other" },
    { RTSP_REPLY_CODE_NOT_MODIFIED, "Not Modified" },
    { RTSP_REPLY_CODE_USER_PROXY, "Use Proxy" },
    { RTSP_REPLY_CODE_BAD_REQUEST, "Bad Request" },
    { RTSP_REPLY_CODE_UNAUTHORIZED, "Unauthorized" },
    { RTSP_REPLY_CODE_PAYMENT_REQUIRED, "Payment Required" },
    { RTSP_REPLY_CODE_FORBIDDEN, "Forbidden" },
    { RTSP_REPLY_CODE_NOT_FOUND, "Not Found" },
    { RTSP_REPLY_CODE_METHOD_NOT_ALLOWED, "Method Not Allowed" },
    { RTSP_REPLY_CODE_NOTACCEPTABLE, "Not Acceptable" },
    { RTSP_REPLY_CODE_PROXY_AUTHENTICATION_REQUIRED, "Proxy Authentication Required" },
    { RTSP_REPLY_CODE_REQUEST_TIME_OUT, "Request Time-out" },
    { RTSP_REPLY_CODE_GONE, "Gone" },
    { RTSP_REPLY_CODE_LENGTH_REQUIRED, "Length Required" },
    { RTSP_REPLY_CODE_PRECONDITION_FAILED, "Precondition Failed" },
    { RTSP_REPLY_CODE_REQUEST_ENTITY_TOO_LARGE, "Request Entity Too Large" },
    { RTSP_REPLY_CODE_URI_TOO_LARGE, "Request-URI Too Large" },
    { RTSP_REPLY_CODE_UNSUPPORTED_MEDIA_TYPE, "Unsupported Media Type" },
    { RTSP_REPLY_CODE_PARAMETER_NOT_UNDERSTOOD, "Parameter Not Understood" },
    { RTSP_REPLY_CODE_CONFERENCE_NOT_FOUND, "Conference Not Found" },
    { RTSP_REPLY_CODE_NOT_ENOUGH_BANDWIDTH, "Not Enough Bandwidth" },
    { RTSP_REPLY_CODE_SESSION_NOT_FOUND, "Session Not Found" },
    { RTSP_REPLY_CODE_METHOD_NOT_VALID, "Method Not Valid in This State" },
    { RTSP_REPLY_CODE_HEADER_FIELD_NOT_VALID, "Header Field Not Valid for Resource" },
    { RTSP_REPLY_CODE_INVALID_RANGE, "InvalidRange" },
    { RTSP_REPLY_CODE_PARAMETER_IS_READ_ONLY, "Parameter Is Read-Only" },
    { RTSP_REPLY_CODE_AGGREGATE_OPERATION_NOT_ALLOWED, "Aggregate Operation Not Allowed" },
    { RTSP_REPLY_CODE_ONLY_AGGREGATE_OPERATION_ALLOWED, "Only Aggregate Operation Allowed" },
    { RTSP_REPLY_CODE_UNSUPPORTED_TRANSPORT, "Unsupported Transport" },
    { RTSP_REPLY_CODE_DESTINATION_UNREACHABLE, "Destination Unreachable" },
    { RTSP_REPLY_CODE_INTERNAL_SERVER_ERROR, "Internal Server Error" },
    { RTSP_REPLY_CODE_NOT_IMPLEMENTED, "Not Implemented" },
    { RTSP_REPLY_CODE_BAD_GATEWAY, "Bad Gateway" },
    { RTSP_REPLY_CODE_SERVICE_UNAVAILABLE, "Service Unavailable" },
    { RTSP_REPLY_CODE_GATEWAY_TIMEOUT, "Gateway Time-out" },
    { RTSP_REPLY_CODE_VERSION_NOT_SUPPORTED, "RTSP Version Not Supported" },
    { RTSP_REPLY_CODE_OPTION_NOT_SUPPORTED, "Option Not Supported" }
};

RtspRemoteRequest::RtspRemoteRequest(pingos::TcpConnection *c, RtspRequestHeader &header, std::string body):
    header(header)
{
    this->connection = c;
    std::string strCSeq = header.GetHeaderValue("cseq");
    if (strCSeq.empty()) {
        MS_THROW_ERROR("Invalid rtsp request, missing cseq");
    }

    this->cseq = std::stoull(strCSeq);
    this->body = body;
}

RtspRemoteRequest::RtspRemoteRequest(const RtspRemoteRequest &request):
    header(request.header)
{
    this->connection = request.connection;
    this->cseq = request.cseq;
    this->body = request.body;
}

RtspRemoteRequest::~RtspRemoteRequest()
{

}

void RtspRemoteRequest::Accept()
{
    RtspHeaderLines lines;

    lines.SetHeaderValue("Server", "PingOS 0.9.1");
    lines.SetHeaderValue("CSeq", std::to_string(this->cseq));
    lines.SetHeaderValue("Content-Lenght", std::to_string(0));

    RtspReplyHeader header(lines);
    header.SetResult(RTSP_REPLY_CODE_OK, RtspReplyCodeString[RTSP_REPLY_CODE_OK]);

    std::string data;
    header.FillString(data);

    this->connection->Send(data, nullptr);
}

void RtspRemoteRequest::Accept(RtspHeaderLines &lines, std::string replyBody)
{
    lines.SetHeaderValue("Server", "PingOS 0.9.1");
    lines.SetHeaderValue("CSeq", std::to_string(this->cseq));
    lines.SetHeaderValue("Content-Length", std::to_string(replyBody.length()));

    RtspReplyHeader header(lines);
    header.SetResult(RTSP_REPLY_CODE_OK, RtspReplyCodeString[RTSP_REPLY_CODE_OK]);

    std::string data;
    header.FillString(data);

    data += replyBody;

    this->connection->Send(data, nullptr);
}

void RtspRemoteRequest::Accept(std::map<std::string, std::string> &replyHeaderLines, std::string replyBody)
{
    RtspHeaderLines lines;

    lines.SetHeaderValue("Server", "PingOS/0.9.1");
    lines.SetHeaderValue("CSeq", std::to_string(this->cseq));

    for (auto it : replyHeaderLines) {
        lines.SetHeaderValue(it.first, it.second);
    }

    lines.SetHeaderValue("Content-Lenght", std::to_string(replyBody.length()));

    RtspReplyHeader header(lines);
    header.SetResult(RTSP_REPLY_CODE_OK, RtspReplyCodeString[RTSP_REPLY_CODE_OK]);

    std::string data;
    header.FillString(data);

    data += replyBody;

    this->connection->Send(data, nullptr);
}

void RtspRemoteRequest::Error(RtspReplyCode code)
{
    RtspHeaderLines lines;

    lines.SetHeaderValue("Server", "PingOS/0.9.1");
    lines.SetHeaderValue("CSeq", std::to_string(this->cseq));
    lines.SetHeaderValue("Content-Lenght", std::to_string(0));

    RtspReplyHeader header(lines);
    header.SetResult(code, RtspReplyCodeString[code]);

    std::string data;
    header.FillString(data);

    this->connection->Send(data, nullptr);
}

}
