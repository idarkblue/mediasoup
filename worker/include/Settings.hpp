#ifndef MS_SETTINGS_HPP
#define MS_SETTINGS_HPP

#include "common.hpp"
#include "LogLevel.hpp"
#include "Channel/Request.hpp"
#include <map>
#include <string>
#include <vector>

class Settings
{
public:
	struct LogTags
	{
		bool info{ false };
		bool ice{ false };
		bool dtls{ false };
		bool rtp{ false };
		bool srtp{ false };
		bool rtcp{ false };
		bool rtx{ false };
		bool bwe{ false };
		bool score{ false };
		bool simulcast{ false };
		bool svc{ false };
		bool sctp{ false };
		bool message{ false };
	};

public:
	// Struct holding the configuration.
	struct Configuration
	{
		std::string confile{ "" };
		LogLevel logLevel{ LogLevel::LOG_ERROR };
		struct LogTags logTags;
		uint16_t rtcMinPort{ 10000u };
		uint16_t rtcMaxPort{ 59999u };
		std::string dtlsCertificateFile;
		std::string dtlsPrivateKeyFile;
	};

public:
	static void SetConfiguration(int argc, char* argv[]);
	static void PrintConfiguration();
	static void HandleRequest(Channel::Request* request);

private:
	static void SetLogLevel(std::string& level);
	static void SetLogTags(const std::vector<std::string>& tags);
	static void SetDtlsCertificateAndPrivateKeyFiles();

public:
	static struct Configuration configuration;

private:
	static std::map<std::string, LogLevel> string2LogLevel;
	static std::map<LogLevel, std::string> logLevel2String;
};

#endif
