#pragma once

#include "json.hpp"

using json = nlohmann::json;

namespace pingos {

class Transport {
public:
    Transport();
    virtual ~Transport();

public:
    int FillJson(json &jsonData);
    int Transform2Sdp(json &jsonData);


};

}
