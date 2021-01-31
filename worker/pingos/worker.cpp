#define MS_CLASS "mediasoup-worker"
// #define MS_LOG_DEV_LEVEL 3

#include "common.hpp"
#include "DepLibSRTP.hpp"
#include "DepLibUV.hpp"
#include "DepLibWebRTC.hpp"
#include "DepOpenSSL.hpp"
#include "DepUsrSCTP.hpp"
#include "Logger.hpp"
#include "MediaSoupErrors.hpp"
#include "Settings.hpp"
#include "Utils.hpp"
#include "Worker.hpp"
#include "Channel/Notifier.hpp"
#include "Channel/UnixStreamSocket.hpp"
#include "PayloadChannel/Notifier.hpp"
#include "PayloadChannel/UnixStreamSocket.hpp"
#include "RTC/DtlsTransport.hpp"
#include "RTC/SrtpSession.hpp"
#include <uv.h>
#include <cerrno>
#include <csignal>  // sigaction()
#include <cstdlib>  // std::_Exit(), std::genenv()
#include <iostream> // std::cerr, std::endl
#include <map>
#include <string>

#include "core/Loop.hpp"
#include "core/Configuration.hpp"
#include "core/Module.hpp"
#include "network/HttpServer.hpp"
#include "network/WssServer.hpp"
#include "modules/ConsoleModule.hpp"
#include "modules/WebRtcModule.hpp"

static constexpr int ConsumerChannelFd{ 3 };
static constexpr int ProducerChannelFd{ 4 };
static constexpr int PayloadConsumerChannelFd{ 5 };
static constexpr int PayloadProducerChannelFd{ 6 };

void IgnoreSignals();

void ModuleInit()
{
    pingos::ConsoleModule::ClassInit();
    pingos::WebRtcModule::ClassInit();
    pingos::Module::ClassInit();
}

int WorkerMain(int argc, char* argv[])
{
	std::string version = std::getenv("MEDIASOUP_VERSION");

	pingos::Loop::ClassInit();

	// Channel socket (it will be handled and deleted by the Worker).
	Channel::UnixStreamSocket* channel{ nullptr };

	// PayloadChannel socket (it will be handled and deleted by the Worker).
	PayloadChannel::UnixStreamSocket* payloadChannel{ nullptr };

	try
	{
		channel = new Channel::UnixStreamSocket(ConsumerChannelFd, ProducerChannelFd);
	}
	catch (const MediaSoupError& error)
	{
		MS_ERROR_STD("error creating the Channel: %s", error.what());

		std::_Exit(EXIT_FAILURE);
	}

	try
	{
		payloadChannel =
		new PayloadChannel::UnixStreamSocket(PayloadConsumerChannelFd, PayloadProducerChannelFd);
	}
	catch (const MediaSoupError& error)
	{
		MS_ERROR_STD("error creating the RTC Channel: %s", error.what());

		std::_Exit(EXIT_FAILURE);
	}

	// Initialize the Logger.
	Logger::ClassInit(channel);

	try
	{
		Settings::SetConfiguration(argc, argv);
	}
	catch (const MediaSoupTypeError& error)
	{
		MS_ERROR_STD("settings error: %s", error.what());

		// 42 is a custom exit code to notify "settings error" to the Node library.
		std::_Exit(42);
	}
	catch (const MediaSoupError& error)
	{
		MS_ERROR_STD("unexpected settings error: %s", error.what());

		std::_Exit(EXIT_FAILURE);
	}

	pingos::Configuration::ClassInit(Settings::configuration.confile);

	MS_DEBUG_TAG(info, "starting mediasoup-worker process [version:%s]", version.c_str());

#if defined(MS_LITTLE_ENDIAN)
	MS_DEBUG_TAG(info, "little-endian CPU detected");
#elif defined(MS_BIG_ENDIAN)
	MS_DEBUG_TAG(info, "big-endian CPU detected");
#else
	MS_WARN_TAG(info, "cannot determine whether little-endian or big-endian");
#endif

#if defined(INTPTR_MAX) && defined(INT32_MAX) && (INTPTR_MAX == INT32_MAX)
	MS_DEBUG_TAG(info, "32 bits architecture detected");
#elif defined(INTPTR_MAX) && defined(INT64_MAX) && (INTPTR_MAX == INT64_MAX)
	MS_DEBUG_TAG(info, "64 bits architecture detected");
#else
	MS_WARN_TAG(info, "cannot determine 32 or 64 bits architecture");
#endif

	Settings::PrintConfiguration();
	DepLibUV::PrintVersion();

	try
	{
		pingos::ConsoleChannel cc;

		// Initialize static stuff.
		DepOpenSSL::ClassInit();
		DepLibSRTP::ClassInit();
		DepUsrSCTP::ClassInit();
		DepLibWebRTC::ClassInit();
		Utils::Crypto::ClassInit();
		RTC::DtlsTransport::ClassInit();
		RTC::SrtpSession::ClassInit();
		Channel::Notifier::ClassInit(&cc);
		PayloadChannel::Notifier::ClassInit(payloadChannel);

		// Ignore some signals.
		IgnoreSignals();

		ModuleInit();
		MS_DEBUG_DEV("starting libuv loop");
		cc.SetEventListener(pingos::ConsoleModule::Get());

		MS_DEBUG_DEV("starting libuv loop");
		pingos::Loop::Run();
		MS_DEBUG_DEV("libuv loop ended");

		// Free static stuff.
		DepLibUV::ClassDestroy();
		DepLibSRTP::ClassDestroy();
		Utils::Crypto::ClassDestroy();
		DepLibWebRTC::ClassDestroy();
		RTC::DtlsTransport::ClassDestroy();
		DepUsrSCTP::ClassDestroy();

		// Wait a bit so peding messages to stdout/Channel arrive to the Node
		// process.
		uv_sleep(200);

		std::_Exit(EXIT_SUCCESS);
	}
	catch (const MediaSoupError& error)
	{
		MS_ERROR("failure exit: %s", error.what());

		std::_Exit(EXIT_FAILURE);
	}
}

void IgnoreSignals()
{
#ifndef _WIN32
	MS_TRACE();

	int err;
	struct sigaction act; // NOLINT(cppcoreguidelines-pro-type-member-init)

	// clang-format off
	std::map<std::string, int> ignoredSignals =
	{
		{ "PIPE", SIGPIPE },
		{ "HUP",  SIGHUP  },
		{ "ALRM", SIGALRM },
		{ "USR1", SIGUSR1 },
		{ "USR2", SIGUSR2 }
	};
	// clang-format on

	act.sa_handler = SIG_IGN; // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
	act.sa_flags   = 0;
	err            = sigfillset(&act.sa_mask);

	if (err != 0)
		MS_THROW_ERROR("sigfillset() failed: %s", std::strerror(errno));

	for (auto& kv : ignoredSignals)
	{
		const auto& sigName = kv.first;
		int sigId           = kv.second;

		err = sigaction(sigId, &act, nullptr);

		if (err != 0)
			MS_THROW_ERROR("sigaction() failed for signal %s: %s", sigName.c_str(), std::strerror(errno));
	}
#endif
}
