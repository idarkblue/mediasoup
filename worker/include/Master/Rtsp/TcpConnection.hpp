#pragma once

#include "common.hpp"
#include "handles/TcpConnection.hpp"
#include "Rtsp/RtspHeader.hpp"

namespace pingos
{
	class TcpConnection : public ::TcpConnection
	{
	public:
		class Listener
		{
		public:
			virtual void OnTcpConnectionPacketReceived(pingos::TcpConnection* connection, RtspHeaderLines &headerLines, std::string body) = 0;
		};

	public:
		TcpConnection(Listener* listener, size_t bufferSize);
		~TcpConnection() override;

		void SetContext(void * ctx);
		void* GetContext();

	public:
		void Send(const uint8_t* data, size_t len, ::TcpConnection::onSendCallback* cb);
		void Send(std::string &data, ::TcpConnection::onSendCallback* cb);
		/* Pure virtual methods inherited from ::TcpConnection. */
	public:
		void UserOnTcpConnectionRead() override;

	private:
		// Passed by argument.
		Listener* listener{ nullptr };
		// Others.
		size_t frameStart{ 0u }; // Where the latest frame starts.
		void* context{ nullptr };
	};
} // namespace RTC
