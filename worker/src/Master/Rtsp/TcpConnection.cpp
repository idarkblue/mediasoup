#define PMS_CLASS "pingos::TcpConnection"
#define MS_CLASS PMS_CLASS

// #define MS_LOG_DEV_LEVEL 3

#include "Rtsp/TcpConnection.hpp"
#include "Log.hpp"
#include "Utils.hpp"
#include <cstring> // std::memmove(), std::memcpy()
#include "Rtsp/RtspHeader.hpp"

namespace pingos
{
	/* Static. */

	static constexpr size_t ReadBufferSize{ 65536 };
	static uint8_t ReadBuffer[ReadBufferSize];

	/* Instance methods. */

	TcpConnection::TcpConnection(Listener* listener, size_t bufferSize)
	  : ::TcpConnection::TcpConnection(bufferSize), listener(listener)
	{
	}

	TcpConnection::~TcpConnection()
	{
	}

	void TcpConnection::SetContext(void * ctx)
	{
		this->context = ctx;
	}

	void* TcpConnection::GetContext()
	{
		return this->context;
	}

	void TcpConnection::UserOnTcpConnectionRead()
	{
		PMS_DEBUG(
		  "data received [local:{} :{}, remote:{} :{}]",
		  GetLocalIp(), GetLocalPort(), GetPeerIp(), GetPeerPort());

		while (true)
		{
			if (IsClosed()) {
				return;
			}

			size_t dataLen = this->bufferDataLen - this->frameStart;
			size_t bodyLen = 0;

			if (dataLen <= 4) {
				return;
			}

			std::string header((char*) this->buffer + this->frameStart, dataLen);
			size_t headerEnd = header.find("\r\n\r\n");
			if (headerEnd == std::string::npos) {
				return;
			}

			size_t headerLength = headerEnd + 4;

			RtspHeaderLines headerLines(header);

			std::string strContentLength = headerLines.GetHeaderValue("content-length");

			if (!strContentLength.empty()) {
				bodyLen = std::stoi(strContentLength);
			}

			// We have bodyLen bytes.
			if (dataLen >= headerLength && dataLen >= headerLength + bodyLen)
			{
				const uint8_t* body = this->buffer + this->frameStart + headerLength;

				// Update received bytes and notify the listener.
				if (bodyLen != 0)
				{
					// Copy the received body into the static buffer so it can be expanded
					// later.
					std::memcpy(ReadBuffer, body, bodyLen);
				}
				std::string strBody((char *)body, bodyLen);
				PMS_DEBUG("rtsp header {}, rtsp body {}", header, strBody);
				if (this->listener) {
					this->listener->OnTcpConnectionPacketReceived(this, headerLines, strBody);
				}

				// If there is no more space available in the buffer and that is because
				// the latest parsed frame filled it, then empty the full buffer.
				if ((this->frameStart + headerLength + bodyLen) == this->bufferSize)
				{
					PMS_DEBUG("no more space in the buffer, emptying the buffer data");

					this->frameStart    = 0;
					this->bufferDataLen = 0;
				}
				// If there is still space in the buffer, set the beginning of the next
				// frame to the next position after the parsed frame.
				else
				{
					this->frameStart += headerLength + bodyLen;
				}

				// If there is more data in the buffer after the parsed frame then
				// parse again. Otherwise break here and wait for more data.
				if (this->bufferDataLen > this->frameStart)
				{
					PMS_DEBUG("there is more data after the parsed frame, continue parsing");

					continue;
				}

				break;
			}

			// Incomplete body.

			// Check if the buffer is full.
			if (this->bufferDataLen == this->bufferSize)
			{
				// First case: the incomplete frame does not begin at position 0 of
				// the buffer, so move the frame to the position 0.
				if (this->frameStart != 0)
				{
					PMS_DEBUG(
					  "no more space in the buffer, moving parsed bytes to the beginning of "
					  "the buffer and wait for more data");

					std::memmove(
					  this->buffer, this->buffer + this->frameStart, this->bufferSize - this->frameStart);
					this->bufferDataLen = this->bufferSize - this->frameStart;
					this->frameStart    = 0;
				}
				// Second case: the incomplete frame begins at position 0 of the buffer.
				// The frame is too big.
				else
				{
					PMS_WARN(
					  "no more space in the buffer for the unfinished frame being parsed, closing the "
					  "connection");

					ErrorReceiving();

					// And exit fast since we are supposed to be deallocated.
					return;
				}
			}
			// The buffer is not full.
			else
			{
				PMS_DEBUG("frame not finished yet, waiting for more data");
			}

			// Exit the parsing loop.
			break;
		}
	}

	void TcpConnection::Send(std::string &data, ::TcpConnection::onSendCallback* cb)
	{
		this->Send((const uint8_t*) data.c_str(), data.length(), cb);
	}

	void TcpConnection::Send(const uint8_t* data, size_t len, ::TcpConnection::onSendCallback* cb)
	{
		::TcpConnection::Write(data, len, cb);
	}
} // namespace pingos
