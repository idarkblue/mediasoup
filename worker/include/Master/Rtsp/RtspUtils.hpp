#pragma once

#include <string>

namespace pingos {

class RtspUtils
{
public:
    static std::string ExtractString(std::string str, std::string start, std::string end);
    static std::string FrontString(std::string str, std::string split);
    static std::string RearString(std::string str, std::string split);
};

}
