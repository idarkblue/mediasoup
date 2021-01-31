#define PMS_CLASS "pingos::TcpClient"
#define MS_CLASS PMS_CLASS

// #define MS_LOG_DEV_LEVEL 3

#include "Rtsp/TcpClient.hpp"
#include "Log.hpp"
#include "Utils.hpp"
#include <cstring> // std::memmove(), std::memcpy()
#include "Rtsp/RtspHeader.hpp"
#include "DepLibUV.hpp"
#include "MediaSoupErrors.hpp"

namespace pingos
{
	/* Static. */

	/* Instance methods. */

	TcpClient::TcpClient(Listener* listener, pingos::TcpConnection::Listener* connListener, size_t bufferSize) :
		connection(connListener, bufferSize)
	{
		this->listener = listener;
	}

	TcpClient::~TcpClient()
	{
	}

	int TcpClient::Connect(const char *ip, uint16_t port)
	{
		struct sockaddr_in addr;
		uv_os_sock_t sock;
		int rc;

		rc = uv_ip4_addr(ip, port, &addr);
		if (rc != 0) {
			PMS_ERROR("uv_ip4_addr({}:{}) failed", ip, port);
			return -1;
		}

		sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
		this->listener = listener;

		PMS_INFO("rtsp tcp connection {}", (void *) this);

		rc = uv_tcp_init(::DepLibUV::GetLoop(), this->connection.GetUvHandle());
		if (rc != 0) {
			PMS_ERROR("uv_ip4_addr({}:{}) tcp init failed", ip, port);
			return -1;
		}

		rc = uv_tcp_open(this->connection.GetUvHandle(), sock);
		if (rc != 0) {
			PMS_ERROR("uv_ip4_addr({}:{}) tcp open failed", ip, port);
			return -1;
		}

		this->connectReq.data = this;

		rc = uv_tcp_connect(&this->connectReq,
							this->connection.GetUvHandle(),
							(const struct sockaddr*) &addr,
							[](uv_connect_t *req, int status) {
								auto *me = (TcpClient*) req->data;
								me->TcpConnectionHandle();
							});

		if (rc != 0) {
			PMS_ERROR("uv_ip4_addr({}:{}) tcp connect failed", ip, port);
			return -1;
		}

		PMS_INFO("uv_ip4_addr({}:{}) tcp connecting", ip, port);

		return 0;
	}

	void TcpClient::Close()
	{
		this->connection.Close();
	}

	TcpConnection* TcpClient::GetTcpConnection()
	{
		return &this->connection;
	}

	void TcpClient::TcpConnectionHandle()
	{
		this->SetLocalAddress();

		try {
			connection.Setup(this, &(this->localAddr), this->localIp, this->localPort, true);
		}
		catch (const MediaSoupError& error)	{
			PMS_ERROR("TcpClient[{}] Setup failed", (void *) this);

			return;
		}

		// Start receiving data.
		try
		{
			// NOTE: This may throw.
			connection.Start();
		}
		catch (const MediaSoupError& error)
		{
			PMS_ERROR("TcpClient[{}] Start failed, reason {}",
				(void *) this, error.what());
			return;
		}

		if (this->listener) {
			this->listener->OnTcpClientConnected(this, &this->connection);
		}
	}

	void TcpClient::OnTcpConnectionClosed(::TcpConnection *connection)
	{
		if (this->listener) {
			this->listener->OnTcpClientClosed(this, &this->connection);
		}
	}

	int TcpClient::SetLocalAddress()
	{
		int err;
		int len = sizeof(this->localAddr);

		err = uv_tcp_getsockname(this->connection.GetUvHandle(),
			reinterpret_cast<struct sockaddr*>(&this->localAddr), &len);

		if (err != 0)
		{
			PMS_ERROR("uv_tcp_getsockname() failed: {}", uv_strerror(err));

			return -1;
		}

		int family;

		Utils::IP::GetAddressInfo(
		reinterpret_cast<const struct sockaddr*>(&this->localAddr), family, this->localIp, this->localPort);

		return 0;
	}
} // namespace pingos
