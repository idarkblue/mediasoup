#pragma once

#include "common.hpp"
#include "Rtsp/TcpConnection.hpp"
#include "Rtsp/RtspHeader.hpp"
#include <uv.h>

namespace pingos
{
	class TcpClient : public ::TcpConnection::Listener
	{
	public:
		class Listener
		{
		public:
			virtual ~Listener() = default;

		public:
			virtual void OnTcpClientConnected(TcpClient *client, pingos::TcpConnection* connection) = 0;
			virtual void OnTcpClientClosed(TcpClient *client, pingos::TcpConnection* connection) = 0;
		};

	public:
		TcpClient(Listener* listener, pingos::TcpConnection::Listener* connListener, size_t bufferSize);
		~TcpClient() override;

		int Connect(const char *ip, uint16_t port);
		void Close();
		TcpConnection* GetTcpConnection();

	protected:
		void TcpConnectionHandle();

	// ::TcpCionnection::Listener
	protected:
		void OnTcpConnectionClosed(::TcpConnection *connection) override;

	protected:
		int SetLocalAddress();

	protected:
		struct sockaddr_storage localAddr;
		std::string localIp;
		uint16_t localPort{ 0u };

	private:
		Listener* listener{ nullptr };
		uv_connect_t connectReq;
		pingos::TcpConnection connection;
	};
} // namespace RTC
