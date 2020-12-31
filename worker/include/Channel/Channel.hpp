#pragma once

#include "Channel/Request.hpp"

namespace Channel
{
    class Channel;

    class Channel
    {
    public:
        class Listener
        {
        public:
            virtual void OnChannelRequest(Channel* channel, Request* request) = 0;
            virtual void OnChannelClosed(Channel* channel) = 0;
        };

    public:
        Channel() = default;
        virtual ~Channel() = default;

    public:
        void SetListener(Listener* listener) {
            this->listener = listener;
        };

        virtual void Send(json& jsonMessage) = 0;

    protected:
        // Passed by argument.
        Listener* listener{ nullptr };
    };
} // namespace Channel
