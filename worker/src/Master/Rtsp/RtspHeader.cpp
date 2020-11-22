#include <algorithm>
#include "Utils.hpp"
#include "Rtsp/RtspHeader.hpp"

namespace pingos {

RtspHeaderLines::RtspHeaderLines(std::string data)
{
    std::string line;
    size_t pos = 0;
    do {
        pos = data.find("\r\n");
        if (pos == std::string::npos) {
            break;
        }
        line = data.substr(0, pos);
        data = data.substr(pos + 2);

        pos = line.find_first_not_of(' ');
        if (pos == std::string::npos) {
            continue;
        }
        line = line.substr(pos);

        Utils::String::ToLowerCase(line);

        if (cmdLine.empty() && (line.find("rtsp/") != std::string::npos)) {
            cmdLine = line;
        } else {
            pos = line.find(':');
            if (pos == std::string::npos) {
                continue;
            }

            std::string key = line.substr(0, pos);
            std::string val = line.substr(pos + 1);
            pos = val.find_first_not_of(' ');
            if (pos == std::string::npos) {
                continue;
            }

            val = val.substr(pos);
            this->lines[key] = val;
        }
    } while (!data.empty());
}

RtspHeaderLines::RtspHeaderLines(const RtspHeaderLines &header)
{
    this->cmdLine = header.cmdLine;
    this->lines = header.lines;
}

RtspHeaderLines::~RtspHeaderLines()
{

}

void RtspHeaderLines::SetHeaderValue(std::string key, std::string value)
{
    this->lines[key] = value;
}

std::string RtspHeaderLines::GetHeaderValue(std::string key)
{
    auto it = this->lines.find(key);
    if (it == this->lines.end()) {
        return "";
    }

    return it->second;
}

void RtspHeaderLines::FillString(std::string &data)
{
    data = this->cmdLine + "\r\n";
    for (auto &it : lines) {
        data += it.first + std::string(": ") + it.second + std::string("\r\n");
    }

    data += std::string("\r\n");
}

void RtspHeaderLines::SplitString(std::string str, char sp, std::vector<std::string> &strVec)
{
    do {
        size_t pos = str.find(sp);
        if (pos == std::string::npos) {
            strVec.push_back(str.substr(0, pos));
            return;
        }

        strVec.push_back(str.substr(0, pos));
        str = str.substr(pos + 1);
    } while (!str.empty());
}

void RtspHeaderLines::SplitString(std::string str, std::string sp, std::vector<std::string> &strVec)
{
    do {
        size_t pos = str.find(sp);
        if (pos == std::string::npos && !str.empty()) {
            strVec.push_back(str.substr(0, pos));
            return;
        }

        if (pos > 0) {
            strVec.push_back(str.substr(0, pos));
        }
        str = str.substr(pos + sp.length());
    } while (!str.empty());
}

std::string RtspHeaderLines::GetSplitValue(std::string str, char sp, std::string key)
{
    std::vector<std::string> strVec;
    RtspHeaderLines::SplitString(str, sp, strVec);
    if (strVec.empty()) {
        return "";
    }

    for (auto &item : strVec) {
        std::vector<std::string> kv;
        RtspHeaderLines::SplitString(item, '=', kv);
        if (kv.size() != 2 || strcasecmp(kv[0].c_str(), key.c_str())) {
            continue;
        }

        return kv[1];
    }

    return "";
}

void RtspHeaderLines::SetCmdLine(std::string cmdLine)
{
    this->cmdLine = cmdLine;
}

RtspRequestHeader::RtspRequestHeader(std::string data)
    : RtspHeaderLines(data)
{

}

RtspRequestHeader::RtspRequestHeader(const RtspHeaderLines &header)
    : RtspHeaderLines(header)
{

}

RtspRequestHeader::RtspRequestHeader(const RtspRequestHeader &header)
    : RtspHeaderLines(header)
{

}

RtspRequestHeader::~RtspRequestHeader()
{

}

void RtspRequestHeader::SetMethod(std::string method, std::string url)
{
    std::string cmdLine;

    std::transform(method.begin(), method.end(), method.begin(), ::tolower);

    cmdLine = method + std::string(" ") + url + std::string(" ") + std::string("RTSP/1.0");

    RtspHeaderLines::SetCmdLine(cmdLine);
}

std::string RtspRequestHeader::GetMethod()
{
    std::string line = this->cmdLine;

    size_t pos = line.find(' ');
    if (pos == std::string::npos) {
        return "";
    }

    std::string method = line.substr(0, pos);

    return method;
}

std::string RtspRequestHeader::GetUrl()
{
    std::string line = this->cmdLine;

    size_t pos = line.find(' ');
    if (pos == std::string::npos) {
        return "";
    }

    std::string url = line.substr(pos + 1);
    pos = url.find_first_not_of(' ');
    if (pos == std::string::npos) {
        return "";
    }

    url = url.substr(pos);

    pos = url.find(' ');
    if (pos != std::string::npos) {
        url = url.substr(0, pos);
    }

    return url;
}

std::string RtspRequestHeader::GetUri()
{
    std::string uri = this->GetUrl();

    size_t pos = uri.find("rtsp://");
    if (pos == std::string::npos) {
        return "";
    }

    uri = uri.substr(pos + strlen("rtsp://"));
    pos = uri.find('?');
    if (pos != std::string::npos) {
        uri = uri.substr(0, pos);
    }

    pos = uri.find('/');
    if (pos == std::string::npos) {
        return "";
    }

    uri = uri.substr(pos);

    return uri;
}

uint16_t RtspRequestHeader::GetPort()
{
    std::string uri = this->GetUrl();

    size_t pos = uri.find("rtsp://");
    if (pos == std::string::npos) {
        return 0;
    }

    uri = uri.substr(pos + strlen("rtsp://"));
    pos = uri.find('?');
    if (pos != std::string::npos) {
        uri = uri.substr(0, pos);
    }

    pos = uri.find(':');
    if (pos == std::string::npos) {
        return 554;
    }

    std::string strPort = uri.substr(pos + 1);
    pos = strPort.find('/');
    if (pos != std::string::npos) {
        strPort.substr(0, pos);
    }

    uint16_t port = std::stoi(strPort);

    return port;
}

std::string RtspRequestHeader::GetHost()
{
    std::string host = this->GetUrl();

    size_t pos = host.find("rtsp://");
    if (pos == std::string::npos) {
        return "";
    }

    host = host.substr(pos + strlen("rtsp://"));
    pos = host.find('?');
    if (pos != std::string::npos) {
        host = host.substr(0, pos);
    }

    pos = host.find('/');
    if (pos != std::string::npos) {
        host.substr(0, pos);
    }

    pos = host.find(':');
    if (pos != std::string::npos) {
        host = host.substr(0, pos);
    }

    return host;
}

std::string RtspRequestHeader::GetArgs()
{
    std::string args = this->GetUrl();

    size_t pos = args.find('?');
    if (pos == std::string::npos) {
        return "";
    }
    args = args.substr(pos + 1);

    return args;
}

std::string RtspRequestHeader::GetArg(std::string key)
{
    std::string args = this->GetArgs();
    if (args.empty()) {
        return "";
    }

    Utils::String::ToLowerCase(key);
    std::vector<std::string> argVec;

    RtspHeaderLines::SplitString(args, '&', argVec);
    for (auto &arg : argVec) {
        std::vector<std::string> kv;
        RtspHeaderLines::SplitString(arg, '=', kv);
        if (kv.size() == 2 && kv[0] == key) {
            return kv[1];
        }
    }

    return "";
}

RtspReplyHeader::RtspReplyHeader(std::string data)
    : RtspHeaderLines(data)
{

}

RtspReplyHeader::RtspReplyHeader(const RtspHeaderLines &header)
    : RtspHeaderLines(header)
{

}

RtspReplyHeader::RtspReplyHeader(const RtspReplyHeader &header)
    : RtspHeaderLines(header)
{

}

RtspReplyHeader::~RtspReplyHeader()
{

}

void RtspReplyHeader::SetResult(int code, std::string reason)
{
    std::string cmdLine;

    cmdLine = std::string("RTSP/1.0") + std::string(" ") + std::to_string(code) + std::string(" ") + reason;

    RtspHeaderLines::SetCmdLine(cmdLine);
}

int RtspReplyHeader::GetResultCode()
{
    return 0;
}

}
