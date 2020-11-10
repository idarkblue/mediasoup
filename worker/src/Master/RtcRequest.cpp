#define PMS_CLASS "pingos::RtcRequest"
#define MS_CLASS "pingos::RtcRequest"

#include "Master/Log.hpp"
#include "MediaSoupErrors.hpp"
#include "Master/RtcRequest.hpp"
#include "Master/Defines.hpp"

namespace pingos {
std::unordered_map<std::string, RtcRequest::MethodId> RtcRequest::string2MethodId =
    {
        { "session.setup", RtcRequest::MethodId::SESSION_SETUP },
        { "stream.publish",    RtcRequest::MethodId::STREAM_PUBLISH },
        { "stream.play",    RtcRequest::MethodId::STREAM_PLAY },
        { "stream.mute",    RtcRequest::MethodId::STREAM_MUTE },
        { "stream.close",    RtcRequest::MethodId::STREAM_CLOSE },
        { "stream.heartbeat",    RtcRequest::MethodId::STREAM_HEARTBEAT }
    };

RtcRequest::RtcRequest(NetConnection *netConnection) : nc(netConnection)
{

}

RtcRequest::RtcRequest(NetConnection *netConnection, json &jsonObject) : nc(netConnection)
{
    this->Parse(jsonObject);
}

RtcRequest::~RtcRequest()
{

}

void RtcRequest::Parse(json &jsonObject)
{
//    JSON_READ_VALUE_THROW(jsonObject, "id", uint64_t, this->id);
    JSON_READ_VALUE_DEFAULT(jsonObject, "method", std::string, this->method, "unknown");
    JSON_READ_VALUE_DEFAULT(jsonObject, "session", std::string, this->session, "");
    JSON_READ_VALUE_DEFAULT(jsonObject, "stream", std::string, this->stream, "unknown");

    if (string2MethodId.find(this->method) == string2MethodId.end()) {
        PMS_ERROR("Unknown method[{}]", this->method);
        MS_THROW_ERROR("Unknown method[%s]", this->method.c_str());
    }

    this->methodId = string2MethodId[this->method];

    auto jsonDataIt = jsonObject.find("data");
    if (jsonDataIt != jsonObject.end()) {
        this->jsonData = *jsonDataIt;
    }
}

int RtcRequest::Accept()
{
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
        PMS_ERROR("StreamId[{}] rtc reply failed", this->stream);
        return -1;
    }

    PMS_DEBUG("StreamId[{}] accept relay success, content {}", this->stream, jsonResponse.dump());

    return 0;
}

int RtcRequest::Accept(json &jsonData)
{
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
        PMS_ERROR("StreamId[{}] rtc reply failed", this->stream);
        return -1;
    }

    PMS_DEBUG("StreamId[{}] accept relay success, content {}", this->stream, jsonResponse.dump());

    return 0;
}

int RtcRequest::Error(const char* reason)
{
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
        PMS_ERROR("StreamId[{}] rtc reply failed", this->stream);
        return -1;
    }

    PMS_DEBUG("StreamId[{}] rtc reply success, content {}", this->stream, jsonResponse.dump());

    return 0;
}

RtcResponse::RtcResponse(NetConnection *nc, std::string sessionId, std::string streamId, std::string method)
{
    this->nc = nc;
    this->session = sessionId;
    this->stream = streamId;
    this->method = method;
}

RtcResponse::~RtcResponse()
{

}

int RtcResponse::Reply(int error, const char* reason, json &jsonData)
{
    json jsonResponse = json::object();

    jsonResponse["version"] = "1.0";
    jsonResponse["method"] = this->method;
    jsonResponse["session"] = this->session;
    jsonResponse["stream"] = this->stream;
    jsonResponse["err"] = error;

    if (reason) {
        jsonResponse["err_msg"] = reason;
    } else {
        jsonResponse["err_msg"] = "error";
    }
    jsonResponse["data"] = jsonData;

    return nc->ReplyString(jsonResponse.dump());
}

}
