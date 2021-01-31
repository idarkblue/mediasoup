#include "utils/JsonHelper.hpp"
#include "ConsoleRequest.hpp"
#include "ConsoleFilter.hpp"
#include "Module.hpp"

#define MS_CLASS "pingos::ConsoleFilter"

namespace pingos
{
    int ConsoleFilter::CreateRouter(std::string routerId)
    {
        json jsonRoot = json::object();
        json jsonInternal = json::object();

        jsonRoot["id"] = 0;
        jsonRoot["method"] = "worker.createRouter";
        jsonInternal["routerId"] = routerId;

        jsonRoot["internal"] = jsonInternal;
        jsonRoot["data"] = json::object();

        ConsoleRequest r(jsonRoot);

        RequestConsoleFunctional(nullptr, r);

        if (r.error != 0) {
            MS_THROW_ERROR("Create router[%s] failed, reason %s",
                routerId.c_str(), r.reason.c_str());
        }

        return 0;
    }

    int ConsoleFilter::CreateWebRtcTransport(Session *s, std::string transportId, json &jsonData, json &jsonResponse)
    {
        json jsonRoot = json::object();
        json jsonInternal = json::object();

        jsonRoot["id"] = 0;
        jsonRoot["method"] = "router.createWebRtcTransport";
        jsonInternal["routerId"] = s->GetStreamId();
        jsonInternal["transportId"] = transportId;

        jsonRoot["internal"] = jsonInternal;
        jsonRoot["data"] = jsonData;

        ConsoleRequest r(jsonRoot);

        RequestConsoleFunctional(s, r);

        if (r.error != 0) {
            MS_THROW_ERROR("Stream[%s] Session[%s], Create WebRtcTransport[%s] failed, reason %s",
                s->GetStreamId().c_str(), s->GetSessionId().c_str(), transportId.c_str(), r.reason.c_str());
        }

        jsonResponse = r.acceptData;

        return 0;
    }

    int ConsoleFilter::ConnectTransport(Session *s, std::string transportId, json &jsonData)
    {
        json jsonRoot = json::object();
        json jsonInternal = json::object();

        jsonRoot["id"] = 0;
        jsonRoot["method"] = "transport.connect";
        jsonInternal["routerId"] = s->GetStreamId();
        jsonInternal["transportId"] = transportId;

        jsonRoot["internal"] = jsonInternal;
        jsonRoot["data"] = jsonData;

        ConsoleRequest r(jsonRoot);

        RequestConsoleFunctional(s, r);

        if (r.error != 0) {
            MS_THROW_ERROR("Stream[%s] Session[%s], Connect Transport[%s] failed, reason %s",
                s->GetStreamId().c_str(), s->GetSessionId().c_str(), transportId.c_str(), r.reason.c_str());
        }

        return 0;
    }

    int ConsoleFilter::CreateProducer(Session *s, std::string transportId, json &jsonData)
    {
        std::string kind;
        JSON_THROW_READ_VALUE(jsonData, "kind", std::string, string, kind);

        json jsonRoot = json::object();
        json jsonInternal = json::object();

        jsonRoot["id"] = 0;
        jsonRoot["method"] = "transport.produce";
        jsonInternal["routerId"] = s->GetStreamId();
        jsonInternal["transportId"] = transportId;
        jsonInternal["producerId"] = kind;

        jsonRoot["internal"] = jsonInternal;
        jsonRoot["data"] = jsonData;

        ConsoleRequest r(jsonRoot);

        RequestConsoleFunctional(s, r);

        if (r.error != 0) {
            MS_THROW_ERROR("Stream[%s] Session[%s], Create Producer[%s] failed, reason %s",
                s->GetStreamId().c_str(), s->GetSessionId().c_str(), kind.c_str(), r.reason.c_str());
        }

        return 0;
    }

    int ConsoleFilter::CreateConsumer(Session *s, std::string transportId, json &jsonData)
    {
        std::string kind;
        JSON_THROW_READ_VALUE(jsonData, "kind", std::string, string, kind);

        json jsonRoot = json::object();
        json jsonInternal = json::object();

        jsonRoot["id"] = 0;
        jsonRoot["method"] = "transport.consume";
        jsonInternal["routerId"] = s->GetStreamId();
        jsonInternal["transportId"] = transportId;
        jsonInternal["consumerId"] = kind;
        jsonInternal["producerId"] = kind;

        jsonRoot["internal"] = jsonInternal;
        jsonRoot["data"] = jsonData;

        ConsoleRequest r(jsonRoot);

        RequestConsoleFunctional(s, r);

        if (r.error != 0) {
            MS_THROW_ERROR("Stream[%s] Session[%s], Create Consumer[%s] failed, reason %s",
                s->GetStreamId().c_str(), s->GetSessionId().c_str(), kind.c_str(), r.reason.c_str());
        }

        return 0;
    }

    int ConsoleFilter::CloseRouter(std::string routerId)
    {
        json jsonRoot = json::object();
        json jsonInternal = json::object();

        jsonRoot["id"] = 0;
        jsonRoot["method"] = "router.close";
        jsonInternal["routerId"] = routerId;

        jsonRoot["internal"] = jsonInternal;
        jsonRoot["data"] = json::object();

        ConsoleRequest r(jsonRoot);

        RequestConsoleFunctional(nullptr, r);

        if (r.error != 0) {
            MS_THROW_ERROR("Close Router[%s] failed, reason %s", routerId.c_str(), r.reason.c_str());
        }

        return 0;
    }

    int ConsoleFilter::CloseTransport(Session *s, std::string transportId)
    {
        json jsonRoot = json::object();
        json jsonInternal = json::object();

        jsonRoot["id"] = 0;
        jsonRoot["method"] = "transport.close";
        jsonInternal["routerId"] = s->GetStreamId();
        jsonInternal["transportId"] = transportId;

        jsonRoot["internal"] = jsonInternal;
        jsonRoot["data"] = json::object();

        ConsoleRequest r(jsonRoot);

        RequestConsoleFunctional(s, r);

        if (r.error != 0) {
            MS_THROW_ERROR("Stream[%s] Session[%s], Close Transport[%s] failed, reason %s",
                s->GetStreamId().c_str(), s->GetSessionId().c_str(), transportId.c_str(), r.reason.c_str());
        }

        return 0;
    }

    int ConsoleFilter::CloseProucer(Session *s, std::string transportId, RTC::Media::Kind kind)
    {
        json jsonRoot = json::object();
        json jsonInternal = json::object();

        jsonRoot["id"] = 0;
        jsonRoot["method"] = "producer.close";
        jsonInternal["routerId"] = s->GetStreamId();
        jsonInternal["transportId"] = transportId;
        jsonInternal["producerId"] = RTC::Media::GetString(kind);

        jsonRoot["internal"] = jsonInternal;
        jsonRoot["data"] = json::object();

        ConsoleRequest r(jsonRoot);

        RequestConsoleFunctional(s, r);

        if (r.error != 0) {
            MS_THROW_ERROR("Stream[%s] Session[%s], Close Producer[%s] failed, reason %s",
                s->GetStreamId().c_str(), s->GetSessionId().c_str(), RTC::Media::GetString(kind).c_str(), r.reason.c_str());
        }

        return 0;
    }

    int ConsoleFilter::CloseConsumer(Session *s, std::string transportId, RTC::Media::Kind kind)
    {
        json jsonRoot = json::object();
        json jsonInternal = json::object();

        jsonRoot["id"] = 0;
        jsonRoot["method"] = "consumer.close";
        jsonInternal["routerId"] = s->GetStreamId();
        jsonInternal["transportId"] = transportId;
        jsonInternal["consumerId"] = RTC::Media::GetString(kind);

        jsonRoot["internal"] = jsonInternal;
        jsonRoot["data"] = json::object();

        ConsoleRequest r(jsonRoot);

        RequestConsoleFunctional(s, r);

        if (r.error != 0) {
            MS_THROW_ERROR("Stream[%s] Session[%s], Close Consumer[%s] failed, reason %s",
                s->GetStreamId().c_str(), s->GetSessionId().c_str(), RTC::Media::GetString(kind).c_str(), r.reason.c_str());
        }

        return 0;
    }

} // namespace pingos
