#ifndef MS_CHANNEL_UNIX_STREAM_SOCKET_HPP
#define MS_CHANNEL_UNIX_STREAM_SOCKET_HPP

#include "common.hpp"
#include "Channel/Request.hpp"
#include "handles/UnixStreamSocket.hpp"
#include "Channel/Channel.hpp"
#include <json.hpp>

namespace Channel
{
	class ConsumerSocket : public ::UnixStreamSocket
	{
	public:
		class Listener
		{
		public:
			virtual void OnConsumerSocketMessage(ConsumerSocket* consumerSocket, char* msg, size_t msgLen) = 0;
			virtual void OnConsumerSocketClosed(ConsumerSocket* consumerSocket) = 0;
		};

	public:
		ConsumerSocket(int fd, size_t bufferSize, Listener* listener);
		ConsumerSocket(uv_pipe_t *handle, size_t bufferSize, Listener* listener);
		/* Pure virtual methods inherited from ::UnixStreamSocket. */
	public:
		void UserOnUnixStreamRead() override;
		void UserOnUnixStreamSocketClosed() override;

	private:
		// Passed by argument.
		Listener* listener{ nullptr };
		// Others.
		size_t msgStart{ 0u }; // Where the latest message starts.
	};

	class ProducerSocket : public ::UnixStreamSocket
	{
	public:
		ProducerSocket(int fd, size_t bufferSize);
		ProducerSocket(uv_pipe_t *handle, size_t bufferSize);

		/* Pure virtual methods inherited from ::UnixStreamSocket. */
	public:
		void UserOnUnixStreamRead() override
		{
		}
		void UserOnUnixStreamSocketClosed() override
		{
		}
	};

	class UnixStreamSocket : public ConsumerSocket::Listener,
							public Channel
	{
	public:
		explicit UnixStreamSocket(int consumerFd, int producerFd);
		virtual ~UnixStreamSocket();

	public:
		virtual void Send(json& jsonMessage) override;
		void SendLog(char* message, size_t messageLen);

	private:
		void SendImpl(const void* nsPayload, size_t nsPayloadLen);

		/* Pure virtual methods inherited from ConsumerSocket::Listener. */
	public:
		void OnConsumerSocketMessage(ConsumerSocket* consumerSocket, char* msg, size_t msgLen) override;
		void OnConsumerSocketClosed(ConsumerSocket* consumerSocket) override;

	private:
		// Others.
		ConsumerSocket consumerSocket;
		ProducerSocket producerSocket;
	};
} // namespace Channel

#endif
