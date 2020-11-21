#include "ChannelRequest.hpp"

namespace pingos {

uint64_t ChannelRequest::requestId = 0;

ChannelRequest::ChannelRequest(const std::string method)
{
    this->Init(method);
}

ChannelRequest::~ChannelRequest()
{

}

void ChannelRequest::Init(const std::string method)
{
    this->id = ChannelRequest::requestId++;
    this->method = method;
    this->jsonInternal = json::object();
    this->jsonData = json::object();

}
void ChannelRequest::SetInternal(json &jsonObject)
{
    this->jsonInternal = jsonObject;
}

void ChannelRequest::SetData(json &jsonObject)
{
    this->jsonData = jsonObject;
}

uint64_t ChannelRequest::GetId()
{
    return this->id;
}

std::string ChannelRequest::GetMethod()
{
    return this->method;
}

void ChannelRequest::FillJson(json &jsonObject)
{
    jsonObject["id"] = this->id;
    jsonObject["method"] = this->method;
    jsonObject["internal"] = this->jsonInternal;
    jsonObject["data"] = this->jsonData;
}


}
