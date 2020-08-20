
#define MS_CLASS "UnixStreamSocket"
// #define MS_LOG_DEV_LEVEL 3

#include "Master/UnixStreamSocket.hpp"
#include "DepLibUV.hpp"
#include "Logger.hpp"
#include "MediaSoupErrors.hpp"
#include <cstring> // std::memcpy()
#include <string_view>
extern "C"
{
#include <netstring.h>
}
/* Static methods for UV callbacks. */

namespace Master {

// netstring length for a 4194304 bytes payload.
static constexpr size_t NsMessageMaxLen{ 4194313 };
static constexpr size_t NsPayloadMaxLen{ 4194304 };
static uint8_t WriteBuffer[NsMessageMaxLen];

/* Instance methods. */
UnixStreamSocket::UnixStreamSocket(uv_pipe_t *handle, Listener* listener, ::UnixStreamSocket::Role role)
    : ::UnixStreamSocket(handle, NsMessageMaxLen, role), m_listener(listener)
{
    MS_TRACE_STD();
}

UnixStreamSocket::~UnixStreamSocket()
{
    MS_TRACE_STD();
}

void UnixStreamSocket::SetListener(Listener* listener)
{
    MS_TRACE_STD();

    this->m_listener = listener;
}

void UnixStreamSocket::Send(json& jsonMessage)
{
    if (this->IsClosed()) {
        return;
    }

    std::string nsPayload = jsonMessage.dump();
    size_t nsPayloadLen   = nsPayload.length();

    if (nsPayloadLen > NsPayloadMaxLen)
    {
        MS_ERROR_STD("mesage too big");

        return;
    }

    SendImpl(nsPayload.c_str(), nsPayloadLen);
}

void UnixStreamSocket::SendBinary(const uint8_t* nsPayload, size_t nsPayloadLen)
{
    if (this->IsClosed())
        return;

    if (nsPayloadLen > NsPayloadMaxLen)
    {
        MS_ERROR_STD("mesage too big");

        return;
    }

    SendImpl(nsPayload, nsPayloadLen);
}

void UnixStreamSocket::SendString(std::string &message)
{
    const uint8_t *nsPayload = (const uint8_t *) message.c_str();
    size_t nsPayloadLen = message.length();

    this->SendBinary(nsPayload, nsPayloadLen);
}

void UnixStreamSocket::SendImpl(const void* nsPayload, size_t nsPayloadLen)
{
    size_t nsNumLen;
    if (nsPayloadLen == 0)
    {
        nsNumLen       = 1;
        WriteBuffer[0] = '0';
        WriteBuffer[1] = ':';
        WriteBuffer[2] = ',';
    }
    else
    {
        nsNumLen = static_cast<size_t>(std::ceil(std::log10(static_cast<double>(nsPayloadLen) + 1)));
        std::sprintf(reinterpret_cast<char*>(WriteBuffer), "%zu:", nsPayloadLen);
        std::memcpy(WriteBuffer + nsNumLen + 1, nsPayload, nsPayloadLen);
        WriteBuffer[nsNumLen + nsPayloadLen + 1] = ',';
    }

    size_t nsLen = nsNumLen + nsPayloadLen + 2;
    this->Write(WriteBuffer, nsLen);
}

void UnixStreamSocket::UserOnUnixStreamRead()
{
    MS_TRACE_STD();

    // Be ready to parse more than a single message in a single TCP chunk.
    while (true)
    {
        if (IsClosed())
            return;

        size_t readLen  = this->bufferDataLen - this->m_msgStart;
        char* payloadStart = nullptr;
        size_t payloadLen;
        int nsRet = netstring_read(
            reinterpret_cast<char*>(this->buffer + this->m_msgStart), readLen, &payloadStart, &payloadLen);

        if (nsRet != 0)
        {
            switch (nsRet)
            {
                case NETSTRING_ERROR_TOO_SHORT:
                {
                    // Check if the buffer is full.
                    if (this->bufferDataLen == this->bufferSize)
                    {
                        // First case: the incomplete message does not begin at position 0 of
                        // the buffer, so move the incomplete message to the position 0.
                        if (this->m_msgStart != 0)
                        {
                            std::memmove(this->buffer, this->buffer + this->m_msgStart, readLen);
                            this->m_msgStart      = 0;
                            this->bufferDataLen = readLen;
                        }
                        // Second case: the incomplete message begins at position 0 of the buffer.
                        // The message is too big, so discard it.
                        else
                        {
                            MS_ERROR_STD(
                                "no more space in the buffer for the unfinished message being parsed, "
                                "discarding it");

                            this->m_msgStart      = 0;
                            this->bufferDataLen = 0;
                        }
                    }

                    // Otherwise the buffer is not full, just wait.
                    return;
                }

                case NETSTRING_ERROR_TOO_LONG:
                {
                    MS_ERROR_STD("NETSTRING_ERROR_TOO_LONG");

                    break;
                }

                case NETSTRING_ERROR_NO_COLON:
                {
                    MS_ERROR_STD("NETSTRING_ERROR_NO_COLON");

                    break;
                }

                case NETSTRING_ERROR_NO_COMMA:
                {
                    MS_ERROR_STD("NETSTRING_ERROR_NO_COMMA");

                    break;
                }

                case NETSTRING_ERROR_LEADING_ZERO:
                {
                    MS_ERROR_STD("NETSTRING_ERROR_LEADING_ZERO");

                    break;
                }

                case NETSTRING_ERROR_NO_LENGTH:
                {
                    MS_ERROR_STD("NETSTRING_ERROR_NO_LENGTH");

                    break;
                }
            }

            // Error, so reset and exit the parsing loop.
            this->m_msgStart      = 0;
            this->bufferDataLen = 0;

            return;
        }

        // If here it means that payloadStart points to the beginning of a JSON string
        // with payloadLen bytes length, so recalculate readLen.
        readLen =
            reinterpret_cast<const uint8_t*>(payloadStart) - (this->buffer + this->m_msgStart) + payloadLen + 1;

        // Notify the listener.
        std::string_view payload {payloadStart, payloadLen};
        this->m_listener->OnChannelMessage(this, payload);

        // If there is no more space available in the buffer and that is because
        // the latest parsed message filled it, then empty the full buffer.
        if ((this->m_msgStart + readLen) == this->bufferSize)
        {
            this->m_msgStart      = 0;
            this->bufferDataLen = 0;
        }
        // If there is still space in the buffer, set the beginning of the next
        // parsing to the next position after the parsed message.
        else
        {
            this->m_msgStart += readLen;
        }

        // If there is more data in the buffer after the parsed message
        // then parse again. Otherwise break here and wait for more data.
        if (this->bufferDataLen > this->m_msgStart)
        {
            continue;
        }

        break;
    }
}

void UnixStreamSocket::UserOnUnixStreamSocketClosed()
{
    MS_TRACE_STD();

    // Notify the listener.
    this->m_listener->OnChannelClosed(this);
}


::UnixStreamSocket::Role UnixStreamSocket::GetRole()
{
    return this->role;
}

} // namespace
