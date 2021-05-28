#pragma once

#include <string>
#include <iostream>
#include <unordered_map>
#include "SubProcess.hpp"

extern "C" {
    #include <uv.h>
}

namespace pingos {

    class RtcWorker : public SubProcess
    {
        public:
            struct Request {
                uint64_t broadcastId { 0 };
                uint64_t id { 0 };
                std::string method { "" };
                json jsonInternal;
                json jsonData;
            };

        public:
            RtcWorker(uv_loop_t *loop);
            virtual ~RtcWorker();

            int SendRequest(Request &request);

        public: // Implement Worker
            virtual void OnProcessMessage(std::string_view &payload) override;

        private:
            void ReceiveChannelAck(json &jsonObject);
            void ReceiveChannelEvent(json &jsonObject);

            std::unordered_map<uint64_t, Request> requestWaittingMap;
    };

}
