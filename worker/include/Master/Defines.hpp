#ifndef _DEFINES_HPP_INCLUDE_
#define _DEFINES_HPP_INCLUDE_

#include <string>
#include <json.hpp>

using json = nlohmann::json;

#ifndef MAX_PATH
#define MAX_PATH 65535
#endif

enum MessageSourceType {
    MS_HTTP,
    MS_CHILD_PIPE
};

struct ConnectionValue {};
struct PlayValue {};
struct PublishValue {};
struct CloseStreamValue {};
struct RtcSession {
    std::string daemon;
    std::string app;
    std::string name;
};

#endif
