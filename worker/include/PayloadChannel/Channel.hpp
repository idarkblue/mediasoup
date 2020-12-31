#pragma once

#include "json.hpp"
#include "PayloadChannel/Request.hpp"
#include "PayloadChannel/Notification.hpp"

using json = nlohmann::json;

namespace PayloadChannel
{
    class Channel;

    class Channel
    {
    public:
        class Listener
        {
        public:
            virtual void OnPayloadChannelNotification(
                Channel* payloadChannel,
                Notification* notification) = 0;
            virtual void OnPayloadChannelRequest(
                Channel* payloadChannel, Request* request) = 0;
            virtual void OnPayloadChannelClosed(Channel* payloadChannel) = 0;
	   };
    public:
        Channel() = default;
        virtual ~Channel() = default;

    public:
        void SetListener(Listener* listener) {
            this->listener = listener;
        };

		virtual void Send(json& jsonMessage, const uint8_t* payload, size_t payloadLen) = 0;
        virtual void Send(json& jsonMessage) = 0;

    protected:
        // Passed by argument.
        Listener* listener{ nullptr };
    };
} // namespace Channel
