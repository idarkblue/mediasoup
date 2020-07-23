#ifndef _MASTER_REQUEST_HPP_INCLUDE_
#define _MASTER_REQUEST_HPP_INCLUDE_

#include <string>
#include <string_view>
#include "Defines.hpp"

namespace Master {

class MasterRequest {
public:
    MasterRequest();
    MasterRequest(MessageSourceType type, bool ssl, void *handle, void* param);
    virtual ~MasterRequest();

    void Reset(MessageSourceType type, bool ssl, void *handle, void* param);
    void Clear();
    void SetUri(std::string_view uri);
    int Padding(std::string_view data);
    const char *GetData();
    int GetDataSize();

    MessageSourceType Type();

private:
    MessageSourceType m_mt;
    bool  m_ssl;
    void *m_handle;
    void *m_param;
    std::string m_receivedData { "" };
};

}

#endif
