#pragma once

#include <string>
#include <json.hpp>

using json = nlohmann::json;

namespace Master {

class ProcessRequest {
public:
    ProcessRequest();
    virtual ~ProcessRequest();

public:
    virtual void FillJson(json& jsonObject);
    virtual std::string ToString();

protected:
    virtual void FillInternalObject() = 0;
    virtual void FillDataObject() = 0;

public:
    uint64_t id;
    std::string method;
    json internal;
    json data;

public:
    static uint64_t idCounter;
};

class CreateRouterRequest : public ProcessRequest {
public:
    CreateRouterRequest(std::string routerId);
    virtual ~CreateRouterRequest();

protected:
    void FillInternalObject() override;
    void FillDataObject() override;

public:
    std::string routerId;
};

}
