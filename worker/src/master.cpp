extern "C" {
    #include <stdio.h>
}
#include <string>
#include "Master/Master.hpp"
#include "Master/Worker.hpp"
#include "Master/HttpServer.hpp"
#include "Master/WssServer.hpp"
#include "Master/RtcMaster.hpp"
#include "Master/Log.hpp"
#include "Master/Loop.hpp"
#include "Master/SdpInfo.hpp"
#include "Master/RtcServer.hpp"
#include "Master/Configuration.hpp"
#include "Master/sdptransform.hpp"

#define PMS_LOG_FILE "./logs/pms.log"

static std::string sdp = R"(
v=0
o=urtc-client 1603159863350814327 2 IN IP4 0.0.0.0
s=-
c=IN IP4 127.0.0.1
t=0 0
a=ice-lite
a=msid-semantic: WMS d0026a4ba79f0a12bc7c07Y24tZ2Q1093463830VP8_1-3126898112
a=group:BUNDLE audio video
m=audio 9 RTP/SAVPF 111
a=rtpmap:111 opus/48000/2
a=fmtp:111 minptime=10;useinbandfec=1
a=rtcp-fb:111 nack
a=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level
a=setup:active
a=mid:audio
a=sendonly
a=ice-ufrag:zcbnnmkjyw9rhnmj
a=ice-pwd:cpsgcf54fd6ce4jc1uxd1gihkabhflst
a=fingerprint:sha-256 ED:BB:50:26:F1:66:50:09:59:AC:8A:E7:81:F0:8C:7A:AE:1B:B4:42:80:DA:5F:DB:80:05:0B:2A:89:36:B9:6F
a=candidate:udpcandidate 1 udp 1078862079 106.75.167.159 26291 typ host
a=ssrc:843551455 cname:hcBJWbcohEvJDMRv
a=ssrc:843551455 msid:d0026a4ba79f0a12bc7c07Y24tZ2Q1093463830VP8_1-3126898112 d0026a4ba79f0a12bc7c07Y24tZ2Q1093463830VP8_1-3126898112_audio
a=ssrc:843551455 mslabel:d0026a4ba79f0a12bc7c07Y24tZ2Q1093463830VP8_1-3126898112
a=ssrc:843551455 label:d0026a4ba79f0a12bc7c07Y24tZ2Q1093463830VP8_1-3126898112_audio
a=candidate:tcpcandidate 1 tcp 1078862079 106.75.167.159 29364 typ host
a=rtcp-mux
a=rtcp-rsize
m=video 9 RTP/SAVPF 96 97
b=AS:800
a=rtpmap:96 VP8/90000
a=rtpmap:97 rtx/90000
a=fmtp:97 apt=96
a=fmtp:97 x-google-min-bitrate=100
a=x-google-flag:conference
a=rtcp-fb:96 goog-remb
a=rtcp-fb:96 ccm fir
a=rtcp-fb:96 nack
a=rtcp-fb:96 nack pli
a=extmap:8 http://www.webrtc.org/experiments/rtp-hdrext/color-space
a=extmap:14 urn:ietf:params:rtp-hdrext:toffset
a=extmap:2 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time
a=extmap:13 urn:3gpp:video-orientation
a=extmap:5 http://www.webrtc.org/experiments/rtp-hdrext/playout-delay
a=extmap:6 http://www.webrtc.org/experiments/rtp-hdrext/video-content-type
a=extmap:7 http://www.webrtc.org/experiments/rtp-hdrext/video-timing
a=setup:active
a=mid:video
a=sendonly
a=ice-ufrag:zcbnnmkjyw9rhnmj
a=ice-pwd:cpsgcf54fd6ce4jc1uxd1gihkabhflst
a=fingerprint:sha-256 ED:BB:50:26:F1:66:50:09:59:AC:8A:E7:81:F0:8C:7A:AE:1B:B4:42:80:DA:5F:DB:80:05:0B:2A:89:36:B9:6F
a=candidate:udpcandidate 1 udp 1078862079 106.75.167.159 26291 typ host
a=candidate:tcpcandidate 1 tcp 1078862079 106.75.167.159 29364 typ host
a=rtcp-mux
a=rtcp-rsize
a=ssrc-group:FID 767184541 476234506
a=ssrc:767184541 cname:hcBJWbcohEvJDMRv
a=ssrc:767184541 msid:d0026a4ba79f0a12bc7c07Y24tZ2Q1093463830VP8_1-3126898112 d0026a4ba79f0a12bc7c07Y24tZ2Q1093463830VP8_1-3126898112_video
a=ssrc:767184541 mslabel:d0026a4ba79f0a12bc7c07Y24tZ2Q1093463830VP8_1-3126898112
a=ssrc:767184541 label:d0026a4ba79f0a12bc7c07Y24tZ2Q1093463830VP8_1-3126898112_video
a=ssrc:476234506 cname:hcBJWbcohEvJDMRv
a=ssrc:476234506 msid:d0026a4ba79f0a12bc7c07Y24tZ2Q1093463830VP8_1-3126898112 d0026a4ba79f0a12bc7c07Y24tZ2Q1093463830VP8_1-3126898112_video
a=ssrc:476234506 mslabel:d0026a4ba79f0a12bc7c07Y24tZ2Q1093463830VP8_1-3126898112
a=ssrc:476234506 label:d0026a4ba79f0a12bc7c07Y24tZ2Q1093463830VP8_1-3126898112_video
)";

static const char *WorkerProcessArgs[] = {
    "--logLevel=debug",
    "--logTags=info",
    "--logTags=ice",
    "--logTags=rtp",
    "--logTags=producer",
    nullptr
};

void test()
{
    json jsonSdp = sdptransform::parse(sdp);
    spdlog::get(PMS_LOGGER)->debug("{}", jsonSdp.dump());
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        printf("Configuration file must be specified.\r\n");
        return 0;
    }

    pingos::Log::ClassInit(PMS_LOG_FILE, spdlog::level::level_enum::trace, spdlog::level::level_enum::trace);
    pingos::Loop::ClassInit();
    pingos::Worker::ClassInit(WorkerProcessArgs, "./logs/pms.sock");
    pingos::Master::ClassInit(pingos::Loop::FetchLoop());
    pingos::Configuration::ClassInit(argv[1]);

    if (pingos::Configuration::Load() != 0) {
        printf("Configuration file load failed.\r\n");
        return 0;
    }

    pingos::WssServer ws;
    if (pingos::Configuration::websocket.ssl) {
        ws.Accept(pingos::Configuration::websocket.port,
            pingos::Configuration::websocket.keyFile,
            pingos::Configuration::websocket.certFile,
            pingos::Configuration::websocket.passPhrase);
    } else {
        ws.Accept(pingos::Configuration::websocket.port);
    }

    pingos::RtcMaster master;

    pingos::RtcServer rtc(&ws, &master);

    pingos::Master::Options opt;
    opt.execDir = pingos::Configuration::master.execPath;
    opt.nWorkers = pingos::Configuration::master.numOfWorkerProcess;
    opt.daemon = false;

    master.StartWorkers(opt);

    test();

    pingos::Loop::Run();
}
