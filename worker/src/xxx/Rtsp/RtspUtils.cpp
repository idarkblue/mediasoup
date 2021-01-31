#include "Rtsp/RtspUtils.hpp"

namespace pingos {

std::string RtspUtils::ExtractString(std::string str, std::string start, std::string end)
{
    auto pos = str.find(start);
    if (pos == std::string::npos) {
        return "";
    }

    str = str.substr(pos + start.length());
    pos = str.find(end);
    if (pos == std::string::npos) {
        return "";
    }

    str = str.substr(0, pos);

    return str;
}

std::string RtspUtils::FrontString(std::string str, std::string split)
{
    auto pos = str.find(split);
    if (pos == std::string::npos) {
        return "";
    }

    str = str.substr(0, pos);

    return str;
}

std::string RtspUtils::RearString(std::string str, std::string split)
{
    auto pos = str.find(split);
    if (pos == std::string::npos) {
        return "";
    }

    str = str.substr(pos + split.length());

    return str;
}

}
