#define MS_CLASS "pingos::Stream"

#include "Logger.hpp"
#include "Stream.hpp"
#include "Module.hpp"
#include "ConsoleFilter.hpp"

namespace pingos
{
    Stream::Stream(std::string streamId) : streamId(streamId)
    {
        ConsoleFilter::CreateRouter(streamId);
    }

    Stream::~Stream()
    {
        if (this->publisher) {
            delete this->publisher;
            this->publisher = nullptr;
        }

        for (auto it = this->players.begin(); it != this->players.end(); ++it) {
            if (it->second) {
                MS_DEBUG("Stream[%s] Session[%s] delete session while invoking ~Stream.",
                    this->streamId.c_str(), it->second->GetSessionId().c_str());
                delete it->second;
            }
        }

        this->players.clear();
        ConsoleFilter::CloseRouter(this->streamId);
    }

    std::string Stream::GetStreamId()
    {
        return this->streamId;
    }

    int Stream::JoinSession(Session *s)
    {
        if (s->IsPublisher()) {
            if (this->publisher) {
                MS_ERROR("Stream[%s] Session[%s] publisher is existed.",
                    this->streamId.c_str(), s->GetSessionId().c_str());
                return -1;
            }
            this->publisher = s;
        } else {
            if (this->players.find(s->GetSessionId()) != this->players.end()) {
                MS_ERROR("Stream[%s] Session[%s] player is existed.",
                    this->streamId.c_str(), s->GetSessionId().c_str());
                return -1;
            }
            this->players[s->GetSessionId()] = s;
        }

        return 0;
    }

    int Stream::RemoveSession(std::string id)
    {
        if (this->publisher && this->publisher->GetSessionId() == id) {
            delete this->publisher;
            this->publisher = nullptr;
        } else {
            auto s = this->FindSession(id);
            if (s != nullptr) {
                delete s;
                this->players.erase(id);
            }
        }

        return 0;
    }

    Session *Stream::FindSession(std::string id)
    {
        if (this->publisher && this->publisher->GetSessionId() == id) {
            return this->publisher;
        }

        auto it = this->players.find(id);
        if (it == this->players.end()) {
            return nullptr;
        }

        return it->second;
    }

    static std::map<std::string, Stream*> g_streams;

    Stream* FetchStream(std::string streamId)
    {
        Stream *stream = nullptr;

        auto it = g_streams.find(streamId);
        if (it == g_streams.end() || !it->second) {
            stream = new Stream(streamId);
            g_streams[streamId] = stream;
        } else {
            stream = it->second;
        }

        return stream;
    }

    Stream* FindStream(std::string streamId)
    {
        Stream *stream = nullptr;

        auto it = g_streams.find(streamId);
        if (it != g_streams.end() && it->second) {
            stream = it->second;
        }

        return stream;
    }

    void RemoveStream(std::string streamId)
    {
        auto it = g_streams.find(streamId);
        if (it != g_streams.end() && it->second) {
            delete it->second;
        }

        g_streams.erase(streamId);
    }

    void RemoveStream(Stream *stream)
    {
        if (stream == nullptr) {
            return;
        }

        RemoveStream(stream->GetStreamId());
    }
}
