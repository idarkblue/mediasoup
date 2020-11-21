#include "common.hpp"
#include "Channel/Request.hpp"
#include "RTC/RTCP/CompoundPacket.hpp"
#include "RTC/RTCP/FeedbackPs.hpp"
#include "RTC/RTCP/FeedbackPsFir.hpp"
#include "RTC/RTCP/FeedbackPsPli.hpp"
#include "RTC/RTCP/FeedbackRtpNack.hpp"
#include "RTC/RTCP/ReceiverReport.hpp"
#include "RTC/RtpDictionaries.hpp"
#include "RTC/RtpHeaderExtensionIds.hpp"
#include "RTC/RtpPacket.hpp"
#include "RTC/RtpStream.hpp"
#include "RTC/RtpStreamSend.hpp"
#include <json.hpp>
#include <string>
#include <unordered_set>
#include <vector>
#include "RtpParametersConstructor.hpp"

using json = nlohmann::json;

namespace pingos {

class ConsumerConstructor
{
public:
    ConsumerConstructor(json &jsonData, RTC::RtpParameters::Type type, RTC::Media::Kind kind);
    virtual ~ConsumerConstructor();

    int TransformConsumer(json &jsonData);

    int FillJson(std::vector<RTC::RtpEncodingParameters> &consumableRtpEncodings, json &jsonData);

public:
    RtpParametersConstructor rtpParameters;
    RTC::RtpParameters::Type type;
    RTC::Media::Kind kind;
    bool paused { false };
    std::string direction { "" };

};

}
