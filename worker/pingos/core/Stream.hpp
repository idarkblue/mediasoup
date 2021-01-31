#pragma once

#include <map>
#include "Session.hpp"

namespace pingos
{
    class Stream {
    public:
        Stream(std::string streamId);
        virtual ~Stream();

        std::string GetStreamId();

        int JoinSession(Session *s);
        int RemoveSession(std::string id);
        Session *FindSession(std::string id);

    private:
        std::string streamId { "" };
        Session *publisher { nullptr };
        std::map<std::string, Session *> players;
    };

    Stream* FetchStream(std::string streamId);
    Stream* FindStream(std::string streamId);
    void RemoveStream(std::string streamId);
    void RemoveStream(Stream *stream);

}
