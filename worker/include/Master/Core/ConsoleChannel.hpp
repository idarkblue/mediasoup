#include "Channel/Channel.hpp"

namespace pingos
{
    class ConsoleChannel : ::Channel::Channel {
    public:
        ConsoleChannel();
        virtual ~ConsoleChannel();

    public:
        virtual void Send(json& jsonMessage) override;
    };
} // namespace pingos
