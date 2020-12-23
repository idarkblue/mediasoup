#pragma once
#include <string>
#include "RtspHeader.hpp"
#include "Rtsp/TcpConnection.hpp"

namespace pingos {

enum RtspReplyCode {
    RTSP_REPLY_CODE_INIT = 0,
    RTSP_REPLY_CODE_CONTINUE = 100,
    RTSP_REPLY_CODE_TIMEOUT = 110,
    RTSP_REPLY_CODE_OK = 200,
    RTSP_REPLY_CODE_CREATED = 201,
    RTSP_REPLY_CODE_LOW_SPACE = 250,
    RTSP_REPLY_CODE_MULTIPLE_CHOICES = 300,
    RTSP_REPLY_CODE_MOVED_PERMANENTLY = 301,
    RTSP_REPLY_CODE_MOVED_TEMP = 302,
    RTSP_REPLY_CODE_SEE_OTHER = 303,
    RTSP_REPLY_CODE_NOT_MODIFIED = 304,
    RTSP_REPLY_CODE_USER_PROXY = 305,
    RTSP_REPLY_CODE_BAD_REQUEST = 400,
    RTSP_REPLY_CODE_UNAUTHORIZED = 401,
    RTSP_REPLY_CODE_PAYMENT_REQUIRED = 402,
    RTSP_REPLY_CODE_FORBIDDEN = 403,
    RTSP_REPLY_CODE_NOT_FOUND = 404,
    RTSP_REPLY_CODE_METHOD_NOT_ALLOWED = 405,
    RTSP_REPLY_CODE_NOTACCEPTABLE = 406,
    RTSP_REPLY_CODE_PROXY_AUTHENTICATION_REQUIRED = 407,
    RTSP_REPLY_CODE_REQUEST_TIME_OUT = 408,
    RTSP_REPLY_CODE_GONE = 410,
    RTSP_REPLY_CODE_LENGTH_REQUIRED = 411,
    RTSP_REPLY_CODE_PRECONDITION_FAILED = 412,
    RTSP_REPLY_CODE_REQUEST_ENTITY_TOO_LARGE = 413,
    RTSP_REPLY_CODE_URI_TOO_LARGE = 414,
    RTSP_REPLY_CODE_UNSUPPORTED_MEDIA_TYPE = 415,
    RTSP_REPLY_CODE_PARAMETER_NOT_UNDERSTOOD = 451,
    RTSP_REPLY_CODE_CONFERENCE_NOT_FOUND = 452,
    RTSP_REPLY_CODE_NOT_ENOUGH_BANDWIDTH = 453,
    RTSP_REPLY_CODE_SESSION_NOT_FOUND = 454,
    RTSP_REPLY_CODE_METHOD_NOT_VALID = 455,
    RTSP_REPLY_CODE_HEADER_FIELD_NOT_VALID = 456,
    RTSP_REPLY_CODE_INVALID_RANGE = 457,
    RTSP_REPLY_CODE_PARAMETER_IS_READ_ONLY = 458,
    RTSP_REPLY_CODE_AGGREGATE_OPERATION_NOT_ALLOWED = 459,
    RTSP_REPLY_CODE_ONLY_AGGREGATE_OPERATION_ALLOWED = 460,
    RTSP_REPLY_CODE_UNSUPPORTED_TRANSPORT = 461,
    RTSP_REPLY_CODE_DESTINATION_UNREACHABLE = 462,
    RTSP_REPLY_CODE_INTERNAL_SERVER_ERROR = 500,
    RTSP_REPLY_CODE_NOT_IMPLEMENTED = 501,
    RTSP_REPLY_CODE_BAD_GATEWAY = 502,
    RTSP_REPLY_CODE_SERVICE_UNAVAILABLE = 503,
    RTSP_REPLY_CODE_GATEWAY_TIMEOUT = 504,
    RTSP_REPLY_CODE_VERSION_NOT_SUPPORTED = 505,
    RTSP_REPLY_CODE_OPTION_NOT_SUPPORTED = 551
};

class RtspRemoteRequest {

public:
    RtspRemoteRequest() = default;
    RtspRemoteRequest(TcpConnection *c, RtspRequestHeader &header, std::string body = "");
    RtspRemoteRequest(const RtspRemoteRequest &request);
    virtual ~RtspRemoteRequest();

public:
    void Accept();
    void Accept(RtspHeaderLines &lines, std::string body = "");
    void Accept(std::map<std::string, std::string> &replyHeaderLines, std::string body = "");
    void Error(RtspReplyCode code);

public:
    TcpConnection *connection { nullptr };
    uint64_t cseq;
    RtspRequestHeader header;
    std::string body;
};

class RtspLocalRequest {
public:
    RtspLocalRequest();
    RtspLocalRequest(TcpConnection *c);
    RtspLocalRequest(const RtspLocalRequest &request);
    RtspLocalRequest(TcpConnection *c, RtspRequestHeader &newHeader, std::string newBody);
    virtual ~RtspLocalRequest();

public:
    int Send(RtspRequestHeader &newHeader, std::string newBody);
    int Send();

public:
    TcpConnection *connection { nullptr };
    uint64_t cseq;
    RtspRequestHeader header;
    std::string body;
};

}
