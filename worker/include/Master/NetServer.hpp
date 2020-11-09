#pragma once

#include <list>
#include <map>
#include <string>
#include <string_view>

namespace pingos {

class NetConnection {
public:
    NetConnection();
    NetConnection(bool ssl, void *handler, void* param);
    virtual ~NetConnection();

public:
    void Reset(bool ssl, void *handler, void* param);
    void Clear();

public:
    void SetUri(std::string uri);
    std::string GetUri();

public:
    int Padding(std::string_view data);
    const char *GetData();
    int GetDataSize();
    void PopData(std::string &data);

public:
    virtual int ReplyBinary(const uint8_t *nsPayload, size_t nsPayloadLen);
    virtual int ReplyString(std::string data);

public:
    bool IsSsl();
    void *GetConnectionHandler();

public:
    void SetSession(void *session);
    void* GetSession();

private:
    std::string m_uri { "" };
    bool  m_ssl { false };
    void *m_handle { nullptr };
    void *m_param { nullptr };
    std::string m_receivedData { "" };
    void *m_session { nullptr };
};

class NetServer {

public:
    class Listener {
    public:
        virtual int OnMessage(NetConnection *nc) = 0;
        virtual void OnDisconnect(NetConnection *nc) = 0;
    };

public:
    NetServer();
    virtual ~NetServer();

    void SetListener(NetServer::Listener *listener);

public:
    virtual int Accept(std::string ip, uint16_t port, std::string location) = 0;
    virtual int Accept(std::string ip, uint16_t port, std::string location, std::string keyfile, std::string certfile, std::string passphrase) = 0;
    virtual int Disconnect(NetConnection *nc) = 0;
    virtual int ReplyBinary(NetConnection *nc, const uint8_t *nsPayload, size_t nsPayloadLen) = 0;
    virtual int ReplyString(NetConnection *nc, std::string data) = 0;

protected:
    int AddConnection(void *handler, NetConnection *nc);
    void RemoveConnection(void *handler);

    NetConnection* GetConnection();
    void PutConnection(NetConnection *msg);

    void RecycleConnection(void *handler);
    NetConnection* FetchConnection(void *handler, bool ssl);

    NetConnection* FindConnection(void *handler, bool ssl);

protected:
    uint16_t port  { 80 };
    uint16_t sslPort { 443 };

    std::map<void *, NetConnection *> ncMap;
    std::list<NetConnection*>         ncFree;
    Listener                         *listener { nullptr };
};

}
