#pragma once

#include <json.hpp>
#include "Session.hpp"

using json = nlohmann::json;

namespace pingos
{
    class RtcConsole {
    public:
        RtcConsole();
        virtual ~RtcConsole();

    public:
        int CreatePlainTransport(Session *s, json &jsonRequest);
        int CreateWebrtcTransport(Session *s);
        int CreatePipeTransport(Session *s);
        int CreateProducer(Session *s);
        int CreateConsumer(Session *s);
    };
} // namespace pingos

