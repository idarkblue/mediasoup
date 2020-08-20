#include "Master/NetServer.hpp"

namespace Master {

NetRequest::NetRequest():
    m_ssl(false), m_handle(nullptr), m_param(nullptr)
{

}

NetRequest::NetRequest(bool ssl, void *handle, void* param):
    m_ssl(ssl), m_handle(handle), m_param(param)
{

}

NetRequest::~NetRequest()
{

}

int NetRequest::ReplyBinary(const uint8_t *nsPayload, size_t nsPayloadLen)
{
    if (m_param == nullptr) {
        return -1;
    }

    NetServer *ns = (NetServer*) m_param;

    return ns->ReplyBinary(this, nsPayload, nsPayloadLen);
}

int NetRequest::ReplyString(std::string data)
{
    if (m_param == nullptr) {
        return -1;
    }

    NetServer *ns = (NetServer*) m_param;

    return ns->ReplyString(this, data);
}

void NetRequest::Reset(bool ssl, void *handle, void* param)
{
    m_ssl = ssl;
    m_handle = handle;
    m_param = param;
    m_receivedData.clear();
}

void NetRequest::Clear()
{
    m_ssl = false;
    m_handle = nullptr;
    m_param = nullptr;
    m_receivedData.clear();
}

int NetRequest::Padding(std::string_view data)
{
    m_receivedData += data;

    return 0;
}

int NetRequest::ParseUri(std::string_view uri)
{
    m_uri = uri;

    auto start = uri.find_first_not_of('/');

    if (start == std::string::npos) {
        return -1;
    }

    auto tmp = uri.substr(start);

    auto pos = tmp.find('/');

    auto app = tmp.substr(0, pos);

    tmp = tmp.substr(pos + 1);

    pos = tmp.find('/');
    if (pos == std::string::npos) {
        return -1;
    }

    auto streamName = tmp.substr(0, pos);

    auto method = tmp.substr(pos + 1);

    if (app.empty() || streamName.empty() || method.empty()) {
        return -1;
    }

    m_method     = method;
    m_app        = app;
    m_streamName = streamName;

    return 0;
}

std::string NetRequest::GetUri()
{
    return m_uri;
}

std::string NetRequest::GetMethod()
{
    return m_method;
}

std::string NetRequest::GetServer()
{
    return m_server;
}

std::string NetRequest::GetApp()
{
    return m_app;
}

std::string NetRequest::GetStreamName()
{
    return m_streamName;
}

const char *NetRequest::GetData()
{
    return m_receivedData.c_str();
}

int NetRequest::GetDataSize()
{
    return m_receivedData.size();
}
bool NetRequest::IsSsl()
{
    return m_ssl;
}

void* NetRequest::GetConnectionHandle()
{
    return m_handle;
}

// class NetRequest end

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

NetRequest* NetServer::GetRequest()
{
    if (m_freeRequests.size() == 0) {
        return new NetRequest();
    }

    NetRequest *msg = m_freeRequests.front();

    m_freeRequests.pop_front();

    return msg;

}

void NetServer::PutRequest(NetRequest *msg)
{
    msg->Clear();
    m_freeRequests.push_back(msg);
}

void NetServer::RemoveRequest(void *handle)
{
    auto it = m_requestMap.find(handle);
    if (it == m_requestMap.end()) {
        return;
    }

    NetRequest *request = it->second;

    if (request != nullptr && m_listener) {
        m_listener->OnAborted(request);
    }

    m_requestMap.erase(it);

    this->PutRequest(request);
}

NetRequest* NetServer::FetchRequest(void *handle, bool ssl)
{
    auto it = m_requestMap.find(handle);
    if (it != m_requestMap.end()) {
        return it->second;
    }

    auto request = this->GetRequest();
    m_requestMap[handle] = request;

    request->Reset(ssl, handle, this);

    return request;
}

}
