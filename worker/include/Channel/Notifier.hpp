#ifndef MS_CHANNEL_NOTIFIER_HPP
#define MS_CHANNEL_NOTIFIER_HPP

#include "common.hpp"
#include <json.hpp>
#include <string>
#include "UnixStreamSocket.hpp"

using json = nlohmann::json;

namespace Channel
{
	class Channel;
	class Notifier
	{
	public:
		static void ClassInit(Channel* channel);
		static void Emit(const std::string& targetId, const char* event);
		static void Emit(const std::string& targetId, const char* event, json& data);

	public:
		// Passed by argument.
		thread_local static UnixStreamSocket* channel;
	};
} // namespace Channel

#endif
