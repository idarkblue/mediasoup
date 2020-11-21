#pragma once

#include <string>
#include <map>
#include <vector>

namespace pingos {
class RtspHeaderLines {
public:
    RtspHeaderLines() = default;
    RtspHeaderLines(std::string data);
    RtspHeaderLines(const RtspHeaderLines &header);
    virtual ~RtspHeaderLines();

    void SetHeaderValue(std::string key, std::string value);
    std::string GetHeaderValue(std::string key);

    void FillString(std::string &data);

public:
    static void SplitString(std::string str, char sp, std::vector<std::string> &strVec);
    static void SplitString(std::string str, std::string sp, std::vector<std::string> &strVec);
    static std::string GetSplitValue(std::string str, char sp, std::string key);

protected:
    void SetCmdLine(std::string cmdLine);

private:
    void ParseHeader(std::string data);

protected:
    std::string cmdLine;
    std::map<std::string, std::string> lines;
};

class RtspRequestHeader :public RtspHeaderLines {
public:
    RtspRequestHeader() = default;
    RtspRequestHeader(std::string data);
    RtspRequestHeader(const RtspHeaderLines &header);
    RtspRequestHeader(const RtspRequestHeader &header);
    virtual ~RtspRequestHeader();

    void SetMethod(std::string method, std::string url);

    std::string GetMethod();
    std::string GetUrl();
    std::string GetUri();
    uint16_t GetPort();
    std::string GetHost();
    std::string GetArgs();
    std::string GetArg(std::string key);
};

class RtspReplyHeader :public RtspHeaderLines {
public:
    RtspReplyHeader(std::string data);
    RtspReplyHeader(const RtspHeaderLines &header);
    RtspReplyHeader(const RtspReplyHeader &header);
    virtual ~RtspReplyHeader();

    void SetResult(int code, std::string reason);

    int GetResultCode();
};

}
