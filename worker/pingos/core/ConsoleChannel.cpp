#include "core/ConsoleChannel.hpp"
#include "Logger.hpp"

#define MS_CLASS "pingos::ConsoleChannel"

namespace pingos
{
    void ConsoleChannel::SetEventListener(EventListener *evl)
    {
        this->evListener = evl;
    }

    void ConsoleChannel::Send(json& jsonMessage)
    {
        MS_DEBUG("send %s", jsonMessage.dump().c_str());
        if (this->evListener) {
            this->evListener->OnChannelEventActived(jsonMessage);
        }

    }

} // namespace pingos
