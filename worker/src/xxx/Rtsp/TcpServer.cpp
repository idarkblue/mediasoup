#define PMS_CLASS "pingos::TcpServer"
// #define MS_LOG_DEV_LEVEL 3

#include "Rtsp/TcpServer.hpp"
#include "Logger.hpp"
#include "Log.hpp"
#include "RTC/PortManager.hpp"
#include <string>

namespace pingos
{
	/* Instance methods. */

	TcpServer::TcpServer(Listener* listener, pingos::TcpConnection::Listener* connListener, std::string& ip, uint16_t port)
	  : // This may throw.
	    ::TcpServer::TcpServer(RTC::PortManager::BindTcp(ip, port), 256), listener(listener),
	    connListener(connListener)
	{
	}

	TcpServer::~TcpServer()
	{
		RTC::PortManager::UnbindTcp(this->localIp, this->localPort);
	}

	void TcpServer::UserOnTcpConnectionAlloc()
	{
		// Allocate a new pingos::TcpConnection for the TcpServer to handle it.
		auto* connection = new pingos::TcpConnection(this->connListener, 65536);
		PMS_DEBUG("Creating rtsp tcp connection[{}]", (void*) connection);

		// Accept it.
		AcceptTcpConnection(connection);
	}

	void TcpServer::UserOnTcpConnectionClosed(::TcpConnection* connection)
	{
		PMS_DEBUG("Closing rtsp tcp connection[{}]", (void*) connection);
		this->listener->OnRtspTcpConnectionClosed(this, static_cast<pingos::TcpConnection*>(connection));
	}
} // namespace pingos
