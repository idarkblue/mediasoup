#include "Master/WorkerRequest.hpp"

namespace pingos {

static std::string JsonStaticStringId("id");
static std::string JsonStaticStringMethod("method");
static std::string JsonStaticStringInternal("internal");
static std::string JsonStaticStringData("data");
static std::string JsonStaticStringRouterId("routerId");

uint64_t ProcessRequest::idCounter(0);

ProcessRequest::ProcessRequest()
{
    this->id = ProcessRequest::idCounter++;
    this->internal = json::object();
    this->data = json::object();
}

ProcessRequest::~ProcessRequest()
{

}

void ProcessRequest::FillJson(json& jsonObject)
{
    this->FillInternalObject();
    this->FillDataObject();

    jsonObject[JsonStaticStringId] = this->id;
    jsonObject[JsonStaticStringMethod] = this->method;
    jsonObject[JsonStaticStringInternal] = this->internal;

    if (!data.empty()) {
        jsonObject[JsonStaticStringData] = this->data;
    }
}

std::string ProcessRequest::ToString()
{
    json jsonObject = json::object();

    this->FillJson(jsonObject);

    return jsonObject.dump();
}

CreateRouterRequest::CreateRouterRequest(std::string routerId)
{
    this->routerId = routerId;
    this->method = "worker.createRouter";
}

CreateRouterRequest::~CreateRouterRequest()
{

}

void CreateRouterRequest::FillInternalObject()
{
    this->internal[JsonStaticStringRouterId] = this->routerId;
}

void CreateRouterRequest::FillDataObject()
{
    return;
}

}