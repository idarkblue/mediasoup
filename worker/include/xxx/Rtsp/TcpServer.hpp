#pragma once

#include "common.hpp"
#include "Rtsp/TcpConnection.hpp"
#include "Rtsp/RtspHeader.hpp"
#include "handles/TcpConnection.hpp"
#include "handles/TcpServer.hpp"
#include <string>

namespace pingos
{
	class TcpServer : public ::TcpServer
	{
	public:
		class Listener
		{
		public:
			virtual void OnRtspTcpConnectionClosed(
			  pingos::TcpServer* tcpServer, pingos::TcpConnection* connection) = 0;
		};

	public:
		TcpServer(Listener* listener, pingos::TcpConnection::Listener* connListener, std::string& ip, uint16_t port);
		virtual ~TcpServer() override;

		/* Pure virtual methods inherited from ::TcpServer. */
	public:
		void UserOnTcpConnectionAlloc() override;
		void UserOnTcpConnectionClosed(::TcpConnection* connection) override;

	private:
		// Passed by argument.
		Listener* listener{ nullptr };
		pingos::TcpConnection::Listener* connListener{ nullptr };
	};
} // namespace RTC
