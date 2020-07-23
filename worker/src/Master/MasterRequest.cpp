#include "Master/MasterRequest.hpp"

namespace Master {

MasterRequest::MasterRequest():
    m_ssl(false), m_handle(nullptr), m_param(nullptr)
{

}

MasterRequest::MasterRequest(MessageSourceType type, bool ssl, void *handle, void* param):
    m_mt(type), m_ssl(ssl), m_handle(handle), m_param(param)
{

}

MasterRequest::~MasterRequest()
{

}

void MasterRequest::Reset(MessageSourceType type, bool ssl, void *handle, void* param)
{
    m_mt = type;
    m_ssl = ssl;
    m_handle = handle;
    m_param = param;
    m_receivedData.clear();
}

void MasterRequest::Clear()
{
    m_ssl = false;
    m_handle = nullptr;
    m_param = nullptr;
    m_receivedData.clear();
}

int MasterRequest::Padding(std::string_view data)
{
    m_receivedData += data;

    return 0;
}

void MasterRequest::SetUri(std::string_view uri)
{

}

const char *MasterRequest::GetData()
{
    return m_receivedData.c_str();
}

int MasterRequest::GetDataSize()
{
    return m_receivedData.size();
}

}
