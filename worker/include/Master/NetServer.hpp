#pragma once

#include <list>
#include <map>
#include <string>
#include <string_view>

namespace Master {

class NetRequest {
public:
    NetRequest();
    NetRequest(bool ssl, void *handle, void* param);
    virtual ~NetRequest();

public:
    void Reset(bool ssl, void *handle, void* param);
    void Clear();

public:
    int ParseUri(std::string_view uri);
    std::string GetUri();
    std::string GetMethod();
    std::string GetServer();
    std::string GetApp();
    std::string GetStreamName();

public:
    int Padding(std::string_view data);
    const char *GetData();
    int GetDataSize();

public:
    virtual int ReplyBinary(const uint8_t *nsPayload, size_t nsPayloadLen);
    virtual int ReplyString(std::string data);

public:
    bool IsSsl();
    void *GetConnectionHandle();

private:
    std::string m_uri { "" };
    bool  m_ssl { false };
    void *m_handle { nullptr };
    void *m_param { nullptr };
    std::string m_receivedData { "" };
    std::string m_method { "" };
    std::string m_server { "" };
    std::string m_app { "" };
    std::string m_streamName { "" };
};

class NetServer {

public:
    class Listener {
    public:
        virtual int OnMessage(NetRequest *request) = 0;
        virtual void OnAborted(NetRequest *request) = 0;
    };

public:
    NetServer();
    virtual ~NetServer();

    void SetListener(NetServer::Listener *listener);

public:
    virtual int ReplyBinary(NetRequest *request, const uint8_t *nsPayload, size_t nsPayloadLen) = 0;
    virtual int ReplyString(NetRequest *request, std::string data) = 0;

protected:
    NetRequest* GetRequest();
    void PutRequest(NetRequest *msg);

    void RemoveRequest(void *handle);
    NetRequest* FetchRequest(void *handle, bool ssl);

protected:
    std::map<void *, NetRequest *> m_requestMap;
    std::list<NetRequest*>         m_freeRequests;
    Listener                      *m_listener { nullptr };
};

}
