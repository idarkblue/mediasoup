#define PMS_CLASS "pingos::RtcWorker"
#define MS_CLASS "pingos::RtcWorker"

#include "Log.hpp"
#include "RtcWorker.hpp"
#include "MediaSoupErrors.hpp"

namespace pingos {

    RtcWorker::RtcWorker(uv_loop_t *loop) : SubProcess(loop)
    {
    }

    RtcWorker::~RtcWorker()
    {
    }

    int RtcWorker::SendRequest(RtcWorker::Request &request)
    {
        json jsonRoot;

        jsonRoot["id"] = request.id;
        jsonRoot["method"] = request.method;
        jsonRoot["internal"] = request.jsonInternal;
        jsonRoot["data"] = request.jsonData;

        if (this->ChannelSend(jsonRoot.dump())) {
            PMS_ERROR("ChannelSend request[{}] failed, content: {}",
                request.id , jsonRoot.dump());
            return -1;
        }

        this->requestWaittingMap[request.id] = request;

        return 0;
    }

    void RtcWorker::OnProcessMessage(std::string_view &payload)
    {
        json jsonObject = json::parse(payload);
        if (jsonObject.find("event") != jsonObject.end()) {
            // receive event
            PMS_WARN("Ignore event {}", payload);
        } else {
            // receive ack
            this->ReceiveChannelAck(jsonObject);
        }
    }

    void RtcWorker::ReceiveChannelAck(json &jsonObject)
    {
        if (jsonObject.find("id") == jsonObject.end()) {
            PMS_ERROR("Invalid channel ack, miss id, data {}", jsonObject.dump());
            return;
        }

        auto id = jsonObject["id"].get<uint64_t>();

        auto it = this->requestWaittingMap.find(id);
        if (it == this->requestWaittingMap.end()) {
            PMS_ERROR("Invalid channel ack, miss request, data {}", jsonObject.dump());
            return;
        }
    }
}
