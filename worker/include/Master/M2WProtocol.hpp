#ifndef _M2W_PROTOCOL_HTTP_INCLUDE_
#define _M2W_PROTOCOL_HTTP_INCLUDE_

#include <string>

namespace Master {

class M2WProtocol {
public:
    M2WProtocol();
    virtual ~M2WProtocol();

private:
    std::string m_receivedData;
};

}

#endif
