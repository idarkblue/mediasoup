#define PMS_CLASS "pingos::RecordRequest"
#define MS_CLASS "pingos::RecordRequest"

#include "Log.hpp"
#include "MediaSoupErrors.hpp"
#include "Record/RecordRequest.hpp"
#include "Defines.hpp"

namespace pingos {
std::unordered_map<std::string, RecordRequest::MethodId> RecordRequest::string2MethodId =
    {
        { "record.start", RecordRequest::MethodId::START_RECORD },
        { "record.stop",    RecordRequest::MethodId::STOP_RECORD }
    };

RecordRequest::RecordRequest(NetConnection *netConnection) : nc(netConnection)
{

}

RecordRequest::RecordRequest(NetConnection *netConnection, json &jsonObject) : nc(netConnection)
{
    this->Parse(jsonObject);
}

RecordRequest::~RecordRequest()
{

}

void RecordRequest::Parse(json &jsonObject)
{
    JSON_READ_VALUE_DEFAULT(jsonObject, "method", std::string, this->method, "unknown");
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

int RecordRequest::Accept()
{
    this->replied = true;

    json jsonResponse = json::object();

    jsonResponse["version"] = "1.0";
    jsonResponse["err"] = 0;
    jsonResponse["err_msg"] = "OK";
    jsonResponse["data"] = json::object();

    if (nc && this->nc->ReplyString(jsonResponse.dump()) != 0) {
        PMS_ERROR("StreamId[{}] rtc reply failed", this->stream);
        return -1;
    }

    PMS_DEBUG("StreamId[{}] accept relay success, content {}", this->stream, jsonResponse.dump());

    return 0;
}

int RecordRequest::Accept(json &jsonData)
{
    this->replied = true;

    json jsonResponse = json::object();

    jsonResponse["version"] = "1.0";
    jsonResponse["err"] = 0;
    jsonResponse["err_msg"] = "OK";
    jsonResponse["data"] = jsonData;

    if (nc && this->nc->ReplyString(jsonResponse.dump()) != 0) {
        PMS_ERROR("StreamId[{}] rtc reply failed", this->stream);
        return -1;
    }

    PMS_DEBUG("StreamId[{}] accept relay success, content {}", this->stream, jsonResponse.dump());

    return 0;
}

int RecordRequest::Error(const char* reason)
{
    this->replied = true;

    json jsonResponse = json::object();

    jsonResponse["version"] = "1.0";
    jsonResponse["err"] = -1;

    if (reason) {
        jsonResponse["err_msg"] = reason;
    } else {
        jsonResponse["err_msg"] = "error";
    }
    jsonResponse["data"] = json::object();

    if (nc && this->nc->ReplyString(jsonResponse.dump()) != 0) {
        PMS_ERROR("StreamId[{}] rtc reply failed", this->stream);
        return -1;
    }

    PMS_DEBUG("StreamId[{}] rtc reply success, content {}", this->stream, jsonResponse.dump());

    return 0;
}

}
