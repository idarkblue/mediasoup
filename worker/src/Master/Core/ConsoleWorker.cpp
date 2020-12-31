#include "ConsoleWorker.hpp"

namespace pingos
{
    ConsoleWorker::ConsoleWorker(::Channel::UnixStreamSocket* channel, PayloadChannel::UnixStreamSocket* payloadChannel)
        : Worker(channel, payloadChannel)
    {

    }

    ConsoleWorker::~ConsoleWorker()
    {

    }

    void ConsoleWorker::ConsoleRequestHandle(ConsoleRequest &request)
    {
        Worker::OnChannelRequest(nullptr, &request);
    }
} // namespace pingos
