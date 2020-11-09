#include "Master/NetServer.hpp"

namespace pingos {

NetConnection::NetConnection():
    m_ssl(false), m_handle(nullptr), m_param(nullptr)
{

}

NetConnection::NetConnection(bool ssl, void *handler, void* param):
    m_ssl(ssl), m_handle(handler), m_param(param)
{

}

NetConnection::~NetConnection()
{

}

int NetConnection::ReplyBinary(const uint8_t *nsPayload, size_t nsPayloadLen)
{
    if (m_param == nullptr) {
        return -1;
    }

    NetServer *ns = (NetServer*) m_param;

    return ns->ReplyBinary(this, nsPayload, nsPayloadLen);
}

int NetConnection::ReplyString(std::string data)
{
    if (m_param == nullptr) {
        return -1;
    }

    NetServer *ns = (NetServer*) m_param;

    return ns->ReplyString(this, data);
}

void NetConnection::Reset(bool ssl, void *handler, void* param)
{
    m_ssl = ssl;
    m_handle = handler;
    m_param = param;
    m_receivedData.clear();
}

void NetConnection::Clear()
{
    m_ssl = false;
    m_handle = nullptr;
    m_param = nullptr;
    m_receivedData.clear();
}

int NetConnection::Padding(std::string_view data)
{
    m_receivedData += data;

    return 0;
}

void NetConnection::SetUri(std::string uri)
{
    m_uri = uri;
}

std::string NetConnection::GetUri()
{
    return m_uri;
}

const char *NetConnection::GetData()
{
    return m_receivedData.c_str();
}

int NetConnection::GetDataSize()
{
    return m_receivedData.size();
}

void NetConnection::PopData(std::string &data)
{
    if (!m_receivedData.empty()) {
        data = m_receivedData;
        m_receivedData.clear();
    }
}

bool NetConnection::IsSsl()
{
    return m_ssl;
}

void* NetConnection::GetConnectionHandler()
{
    return m_handle;
}

void NetConnection::SetSession(void *session)
{
    m_session = session;
}

void* NetConnection::GetSession()
{
    return m_session;
}

// class NetConnection end

// class NetServer begin
NetServer::NetServer()
{

}

NetServer::~NetServer()
{

}

void NetServer::SetListener(NetServer::Listener *listener)
{
    m_listener = listener;
}

int NetServer::AddConnection(void *handler, NetConnection *nc)
{
    auto it = m_ncMap.find(handler);
    if (it != m_ncMap.end()) {
        return -1;
    }

    m_ncMap[handler] = nc;

    return 0;
}

void NetServer::RemoveConnection(void *handler)
{
    m_ncMap.erase(handler);
}

NetConnection* NetServer::GetConnection()
{
    if (m_ncFree.size() == 0) {
        return new NetConnection();
    }

    NetConnection *msg = m_ncFree.front();

    m_ncFree.pop_front();

    return msg;

}

void NetServer::PutConnection(NetConnection *msg)
{
    msg->Clear();
    m_ncFree.push_back(msg);
}

void NetServer::RecycleConnection(void *handler)
{
    auto it = m_ncMap.find(handler);
    if (it == m_ncMap.end()) {
        return;
    }

    NetConnection *nc = it->second;

    if (nc != nullptr && m_listener) {
        m_listener->OnDisconnect(nc);
    }

    m_ncMap.erase(it);

    this->PutConnection(nc);
}

NetConnection* NetServer::FetchConnection(void *handler, bool ssl)
{
    auto it = m_ncMap.find(handler);
    if (it != m_ncMap.end()) {
        return it->second;
    }

    auto nc = this->GetConnection();
    m_ncMap[handler] = nc;

    nc->Reset(ssl, handler, this);

    return nc;
}

NetConnection* NetServer::FindConnection(void *handler, bool ssl)
{
    auto it = m_ncMap.find(handler);
    if (it == m_ncMap.end()) {
        return nullptr;
    }

    return it->second;
}

}
