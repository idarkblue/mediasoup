#pragma once

namespace pingos {

class WebRtcTransport {
public:
    WebRtcTransport();
    virtual ~WebRtcTransport();

    int Initialize(std::string sdp);

private:
    int Transform;
};

}
