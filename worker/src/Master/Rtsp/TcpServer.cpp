#define MS_CLASS "pingos::TcpServer"
// #define MS_LOG_DEV_LEVEL 3

#include "Rtsp/TcpServer.hpp"
#include "Logger.hpp"
#include "RTC/PortManager.hpp"
#include <string>

namespace pingos
{
	/* Static. */

	static constexpr size_t MaxTcpConnectionsPerServer{ 10 };

	/* Instance methods. */

	TcpServer::TcpServer(Listener* listener, pingos::TcpConnection::Listener* connListener, std::string& ip, uint16_t port)
	  : // This may throw.
	    ::TcpServer::TcpServer(RTC::PortManager::BindTcp(ip, port), 256), listener(listener),
	    connListener(connListener)
	{
		MS_TRACE();
	}

	TcpServer::~TcpServer()
	{
		MS_TRACE();

		RTC::PortManager::UnbindTcp(this->localIp, this->localPort);
	}

	void TcpServer::UserOnTcpConnectionAlloc()
	{
		MS_TRACE();

		// Allow just MaxTcpConnectionsPerServer.
		if (GetNumConnections() >= MaxTcpConnectionsPerServer)
		{
			MS_ERROR("cannot handle more than %zu connections", MaxTcpConnectionsPerServer);

			return;
		}

		// Allocate a new pingos::TcpConnection for the TcpServer to handle it.
		auto* connection = new pingos::TcpConnection(this->connListener, 65536);

		// Accept it.
		AcceptTcpConnection(connection);
	}

	void TcpServer::UserOnTcpConnectionClosed(::TcpConnection* connection)
	{
		MS_TRACE();

		this->listener->OnRtspTcpConnectionClosed(this, static_cast<pingos::TcpConnection*>(connection));
	}
} // namespace pingos
