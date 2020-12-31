#ifndef MS_WORKER_HPP
#define MS_WORKER_HPP

#include "common.hpp"
#include "Channel/Request.hpp"
#include "Channel/UnixStreamSocket.hpp"
#include "PayloadChannel/Notification.hpp"
#include "PayloadChannel/Request.hpp"
#include "PayloadChannel/UnixStreamSocket.hpp"
#include "RTC/Router.hpp"
#include "handles/SignalsHandler.hpp"
#include <json.hpp>
#include <string>
#include <unordered_map>

using json = nlohmann::json;

class Worker : public Channel::Channel::Listener,
               public PayloadChannel::Channel::Listener,
               public SignalsHandler::Listener
{
public:
	explicit Worker(Channel::Channel* channel, PayloadChannel::Channel* payloadChannel);
	~Worker();

private:
	void Close();
	void FillJson(json& jsonObject) const;
	void FillJsonResourceUsage(json& jsonObject) const;
	void SetNewRouterIdFromInternal(json& internal, std::string& routerId) const;
	RTC::Router* GetRouterFromInternal(json& internal) const;

	/* Methods inherited from Channel::Channel::Listener. */
public:
	void OnChannelRequest(Channel::Channel* channel, Channel::Request* request) override;
	void OnChannelClosed(Channel::Channel* channel) override;

	/* Methods inherited from PayloadChannel::UnixStreamSocket::Listener. */
public:
	void OnPayloadChannelNotification(
	  PayloadChannel::Channel* payloadChannel,
	  PayloadChannel::Notification* notification) override;
	void OnPayloadChannelRequest(
	  PayloadChannel::Channel* payloadChannel, PayloadChannel::Request* request) override;
	void OnPayloadChannelClosed(PayloadChannel::Channel* payloadChannel) override;

	/* Methods inherited from SignalsHandler::Listener. */
public:
	void OnSignal(SignalsHandler* signalsHandler, int signum) override;

private:
	// Passed by argument.
	Channel::Channel* channel{ nullptr };
	PayloadChannel::Channel* payloadChannel{ nullptr };
	// Allocated by this.
	SignalsHandler* signalsHandler{ nullptr };
	std::unordered_map<std::string, RTC::Router*> mapRouters;
	// Others.
	bool closed{ false };
};

#endif
