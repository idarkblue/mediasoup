#pragma once

#include "json.hpp"
#include "Session.hpp"

using json = nlohmann::json;

namespace pingos
{
    class ConsoleFilter {
    public:
        ConsoleFilter() = default;
        virtual ~ConsoleFilter() = default;

    public:
        static int CreateRouter(std::string routerId);
        static int CreateWebRtcTransport(Session *s, std::string transportId, json &jsonData, json &jsonResponse);
        static int ConnectTransport(Session *s, std::string transportId, json &jsonData);
        static int CreateProducer(Session *s, std::string transportId, json &jsonData);
        static int CreateConsumer(Session *s, std::string transportId, json &jsonData);

        static int CloseRouter(std::string routerId);
        static int CloseTransport(Session *s, std::string transportId);
        static int CloseProucer(Session *s, std::string transportId, RTC::Media::Kind kind);
        static int CloseConsumer(Session *s, std::string transportId, RTC::Media::Kind kind);
    };
} // namespace pingos
