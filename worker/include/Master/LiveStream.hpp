#ifndef _LIVE_STREAM_HPP_INCLUDE_
#define _LIVE_STREAM_HPP_INCLUDE_

#include <string>
#include <list>

namespace Master {

class LiveSession {

protected:
    std::string m_server { "" };
    std::string m_app { "" };
    std::string m_name { "" };
    std::string m_sessionId { "" };
};

class LiveStream {
public:
    LiveStream();
    ~LiveStream();

public:
    int Join(LiveSession *s, bool publishing);

private:
    std::string m_server { "" };
    std::string m_app { "" };
    std::string m_name { "" };
    LiveSession *m_publisher;
    std::list<LiveSession*> m_players;
};

} // namespace Master


#endif
