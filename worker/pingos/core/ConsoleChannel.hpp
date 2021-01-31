#include "Channel/Channel.hpp"

namespace pingos
{
    class ConsoleChannel : public ::Channel::Channel {
    public:
        class EventListener {
        public:
            virtual void OnChannelEventActived(json &jsonMessage) = 0;
        };

    public:
        ConsoleChannel() = default;
        virtual ~ConsoleChannel() = default;

    public:
        void SetEventListener(EventListener *evl);
        virtual void Send(json& jsonMessage) override;

    private:
        EventListener *evListener { nullptr };
    };
} // namespace pingos
