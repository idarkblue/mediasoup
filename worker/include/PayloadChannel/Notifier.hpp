#ifndef MS_PAYLOAD_CHANNEL_NOTIFIER_HPP
#define MS_PAYLOAD_CHANNEL_NOTIFIER_HPP

#include "common.hpp"
#include <json.hpp>
#include <string>

using json = nlohmann::json;

namespace PayloadChannel
{
	class Channel;
	class Notifier
	{
	public:
		static void ClassInit(Channel* payloadChannel);
		static void Emit(
		  const std::string& targetId, const char* event, const uint8_t* payload, size_t payloadLen);
		static void Emit(
		  const std::string& targetId,
		  const char* event,
		  json& data,
		  const uint8_t* payload,
		  size_t payloadLen);

	public:
		// Passed by argument.
		static Channel* payloadChannel;
	};
} // namespace PayloadChannel

#endif
