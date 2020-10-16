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

#define PMS_LOG_FILE "./logs/pms.log"

static const char *WorkerProcessArgs[] = {
    "--logLevel=debug",
    "--logTags=info",
    "--logTags=ice",
    "--logTags=rtp",
    nullptr
};

static std::string sdp0 = R"(
v=0
o=- 3419650625782168933 2 IN IP4 127.0.0.1
s=-
t=0 0
a=group:BUNDLE 0 1 2
a=msid-semantic: WMS
m=audio 9 UDP/TLS/RTP/SAVPF 111 103 104 9 102 0 8 106 105 13 110 112 113 126
c=IN IP4 0.0.0.0
a=rtcp:9 IN IP4 0.0.0.0
a=ice-ufrag:cJGn
a=ice-pwd:eUcvCWBeF5Pg/1xkCZXSCQ8i
a=ice-options:trickle
a=fingerprint:sha-256 1C:93:EA:12:7D:63:C3:31:B4:99:4A:36:DC:BE:7A:B2:56:23:84:0B:AB:D6:9C:12:23:52:FE:1B:16:82:5E:B6
a=setup:active
a=mid:0
a=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level
a=extmap:9 urn:ietf:params:rtp-hdrext:sdes:mid
a=sendrecv
a=msid:- 206ff53c-07d5-4025-9acb-d781c11ba3b9
a=rtcp-mux
a=rtpmap:111 opus/48000/2
a=rtcp-fb:111 transport-cc
a=fmtp:111 minptime=10;useinbandfec=1
a=rtpmap:103 ISAC/16000
a=rtpmap:104 ISAC/32000
a=rtpmap:9 G722/8000
a=rtpmap:102 ILBC/8000
a=rtpmap:0 PCMU/8000
a=rtpmap:8 PCMA/8000
a=rtpmap:106 CN/32000
a=rtpmap:105 CN/16000
a=rtpmap:13 CN/8000
a=rtpmap:110 telephone-event/48000
a=rtpmap:112 telephone-event/32000
a=rtpmap:113 telephone-event/16000
a=rtpmap:126 telephone-event/8000
a=ssrc:3531964835 cname:0aCTFFN8Z+MmbV1U
m=video 9 UDP/TLS/RTP/SAVPF 96 97 98 99 100 101 127 124 125
c=IN IP4 0.0.0.0
a=rtcp:9 IN IP4 0.0.0.0
a=ice-ufrag:cJGn
a=ice-pwd:eUcvCWBeF5Pg/1xkCZXSCQ8i
a=ice-options:trickle
a=fingerprint:sha-256 1C:93:EA:12:7D:63:C3:31:B4:99:4A:36:DC:BE:7A:B2:56:23:84:0B:AB:D6:9C:12:23:52:FE:1B:16:82:5E:B6
a=setup:active
a=mid:1
a=extmap:2 urn:ietf:params:rtp-hdrext:toffset
a=extmap:3 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time
a=extmap:4 urn:3gpp:video-orientation
a=extmap:5 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01
a=extmap:6 http://www.webrtc.org/experiments/rtp-hdrext/playout-delay
a=extmap:7 http://www.webrtc.org/experiments/rtp-hdrext/video-content-type
a=extmap:8 http://www.webrtc.org/experiments/rtp-hdrext/video-timing
a=extmap:10 http://tools.ietf.org/html/draft-ietf-avtext-framemarking-07
a=extmap:9 urn:ietf:params:rtp-hdrext:sdes:mid
a=inactive
a=rtcp-mux
a=rtcp-rsize
a=rtpmap:96 VP8/90000
a=rtcp-fb:96 goog-remb
a=rtcp-fb:96 transport-cc
a=rtcp-fb:96 ccm fir
a=rtcp-fb:96 nack
a=rtcp-fb:96 nack pli
a=rtpmap:97 rtx/90000
a=fmtp:97 apt=96
a=rtpmap:98 VP9/90000
a=rtcp-fb:98 goog-remb
a=rtcp-fb:98 transport-cc
a=rtcp-fb:98 ccm fir
a=rtcp-fb:98 nack
a=rtcp-fb:98 nack pli
a=fmtp:98 x-google-profile-id=0
a=rtpmap:99 rtx/90000
a=fmtp:99 apt=98
a=rtpmap:100 multiplex/90000
a=rtcp-fb:100 goog-remb
a=rtcp-fb:100 transport-cc
a=rtcp-fb:100 ccm fir
a=rtcp-fb:100 nack
a=rtcp-fb:100 nack pli
a=fmtp:100 acn=VP9;x-google-profile-id=0
a=rtpmap:101 rtx/90000
a=fmtp:101 apt=100
a=rtpmap:127 red/90000
a=rtpmap:124 rtx/90000
a=fmtp:124 apt=127
a=rtpmap:125 ulpfec/90000
m=application 9 DTLS/SCTP 5000
c=IN IP4 0.0.0.0
b=AS:30
a=ice-ufrag:cJGn
a=ice-pwd:eUcvCWBeF5Pg/1xkCZXSCQ8i
a=ice-options:trickle
a=fingerprint:sha-256 1C:93:EA:12:7D:63:C3:31:B4:99:4A:36:DC:BE:7A:B2:56:23:84:0B:AB:D6:9C:12:23:52:FE:1B:16:82:5E:B6
a=setup:active
a=mid:2
a=sctpmap:5000 webrtc-datachannel 1024
)";

static std::string pubOfferSdp = R"(v=0
o=- 219483042265655866 2 IN IP4 127.0.0.1
s=-
t=0 0
a=group:BUNDLE audio video
a=msid-semantic: WMS H2m2Rc4dEpInDChmqBlBYLxgJgsFBPihZkip
m=audio 9 UDP/TLS/RTP/SAVPF 111 103 104 9 0 8 106 105 13 110 112 113 126
c=IN IP4 0.0.0.0
a=rtcp:9 IN IP4 0.0.0.0
a=ice-ufrag:r3sd
a=ice-pwd:nqhP10PVl/HnRVGaznNdOZ1K
a=ice-options:trickle
a=fingerprint:sha-256 2B:EB:31:CA:C7:95:38:6A:60:FB:FF:FE:85:8C:C3:66:E3:57:59:B9:B2:2C:BF:DD:E6:A4:44:14:86:BA:4C:4D
a=setup:actpass
a=mid:audio
a=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level
a=extmap:2 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time
a=extmap:3 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01
a=sendrecv
a=rtcp-mux
a=rtpmap:111 opus/48000/2
a=fmtp:111 minptime=10;useinbandfec=1
a=rtpmap:103 ISAC/16000
a=rtpmap:104 ISAC/32000
a=rtpmap:9 G722/8000
a=rtpmap:0 PCMU/8000
a=rtpmap:8 PCMA/8000
a=rtpmap:106 CN/32000
a=rtpmap:105 CN/16000
a=rtpmap:13 CN/8000
a=rtpmap:110 telephone-event/48000
a=rtpmap:112 telephone-event/32000
a=rtpmap:113 telephone-event/16000
a=rtpmap:126 telephone-event/8000
a=ssrc:2893198208 cname:FsEVKwE8CwUdvojG
a=ssrc:2893198208 msid:H2m2Rc4dEpInDChmqBlBYLxgJgsFBPihZkip 725e9251-8b41-4bcf-b412-138729e3a334
a=ssrc:2893198208 mslabel:H2m2Rc4dEpInDChmqBlBYLxgJgsFBPihZkip
a=ssrc:2893198208 label:725e9251-8b41-4bcf-b412-138729e3a334
m=video 9 UDP/TLS/RTP/SAVPF 96 97 98 99 100 101 102 121 127 120 125 107 108 109 124 119 123 118 114 115 116
c=IN IP4 0.0.0.0
a=rtcp:9 IN IP4 0.0.0.0
a=ice-ufrag:r3sd
a=ice-pwd:nqhP10PVl/HnRVGaznNdOZ1K
a=ice-options:trickle
a=fingerprint:sha-256 2B:EB:31:CA:C7:95:38:6A:60:FB:FF:FE:85:8C:C3:66:E3:57:59:B9:B2:2C:BF:DD:E6:A4:44:14:86:BA:4C:4D
a=setup:actpass
a=mid:video
a=extmap:14 urn:ietf:params:rtp-hdrext:toffset
a=extmap:2 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time
a=extmap:13 urn:3gpp:video-orientation
a=extmap:3 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01
a=extmap:5 http://www.webrtc.org/experiments/rtp-hdrext/playout-delay
a=extmap:6 http://www.webrtc.org/experiments/rtp-hdrext/video-content-type
a=extmap:7 http://www.webrtc.org/experiments/rtp-hdrext/video-timing
a=extmap:8 http://www.webrtc.org/experiments/rtp-hdrext/color-space
a=sendrecv
a=rtcp-mux
a=rtcp-rsize
a=rtpmap:96 VP8/90000
a=rtcp-fb:96 goog-remb
a=rtcp-fb:96 ccm fir
a=rtcp-fb:96 nack
a=rtcp-fb:96 nack pli
a=rtpmap:97 rtx/90000
a=fmtp:97 apt=96
a=rtpmap:98 VP9/90000
a=rtcp-fb:98 goog-remb
a=rtcp-fb:98 ccm fir
a=rtcp-fb:98 nack
a=rtcp-fb:98 nack pli
a=fmtp:98 profile-id=0
a=rtpmap:99 rtx/90000
a=fmtp:99 apt=98
a=rtpmap:100 VP9/90000
a=rtcp-fb:100 goog-remb
a=rtcp-fb:100 ccm fir
a=rtcp-fb:100 nack
a=rtcp-fb:100 nack pli
a=fmtp:100 profile-id=2
a=rtpmap:101 rtx/90000
a=fmtp:101 apt=100
a=rtpmap:102 H264/90000
a=rtcp-fb:102 goog-remb
a=rtcp-fb:102 ccm fir
a=rtcp-fb:102 nack
a=rtcp-fb:102 nack pli
a=fmtp:102 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=42001f
a=rtpmap:121 rtx/90000
a=fmtp:121 apt=102
a=rtpmap:127 H264/90000
a=rtcp-fb:127 goog-remb
a=rtcp-fb:127 ccm fir
a=rtcp-fb:127 nack
a=rtcp-fb:127 nack pli
a=fmtp:127 level-asymmetry-allowed=1;packetization-mode=0;profile-level-id=42001f
a=rtpmap:120 rtx/90000
a=fmtp:120 apt=127
a=rtpmap:125 H264/90000
a=rtcp-fb:125 goog-remb
a=rtcp-fb:125 ccm fir
a=rtcp-fb:125 nack
a=rtcp-fb:125 nack pli
a=fmtp:125 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=42e01f
a=rtpmap:107 rtx/90000
a=fmtp:107 apt=125
a=rtpmap:108 H264/90000
a=rtcp-fb:108 goog-remb
a=rtcp-fb:108 ccm fir
a=rtcp-fb:108 nack
a=rtcp-fb:108 nack pli
a=fmtp:108 level-asymmetry-allowed=1;packetization-mode=0;profile-level-id=42e01f
a=rtpmap:109 rtx/90000
a=fmtp:109 apt=108
a=rtpmap:124 H264/90000
a=rtcp-fb:124 goog-remb
a=rtcp-fb:124 ccm fir
a=rtcp-fb:124 nack
a=rtcp-fb:124 nack pli
a=fmtp:124 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=4d0032
a=rtpmap:119 rtx/90000
a=fmtp:119 apt=124
a=rtpmap:123 H264/90000
a=rtcp-fb:123 goog-remb
a=rtcp-fb:123 ccm fir
a=rtcp-fb:123 nack
a=rtcp-fb:123 nack pli
a=fmtp:123 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=640032
a=rtpmap:118 rtx/90000
a=fmtp:118 apt=123
a=rtpmap:114 red/90000
a=rtpmap:115 rtx/90000
a=fmtp:115 apt=114
a=rtpmap:116 ulpfec/90000
a=ssrc-group:FID 4173810748 781471202
a=ssrc:4173810748 cname:FsEVKwE8CwUdvojG
a=ssrc:4173810748 msid:H2m2Rc4dEpInDChmqBlBYLxgJgsFBPihZkip 7b8fb339-9ffe-4d28-874e-6fa35fa32244
a=ssrc:4173810748 mslabel:H2m2Rc4dEpInDChmqBlBYLxgJgsFBPihZkip
a=ssrc:4173810748 label:7b8fb339-9ffe-4d28-874e-6fa35fa32244
a=ssrc:781471202 cname:FsEVKwE8CwUdvojG
a=ssrc:781471202 msid:H2m2Rc4dEpInDChmqBlBYLxgJgsFBPihZkip 7b8fb339-9ffe-4d28-874e-6fa35fa32244
a=ssrc:781471202 mslabel:H2m2Rc4dEpInDChmqBlBYLxgJgsFBPihZkip
a=ssrc:781471202 label:7b8fb339-9ffe-4d28-874e-6fa35fa32244
)";

static std::string egSdp = R"(
v=0
o=- 235914367549117899 2 IN IP4 127.0.0.1
s=-
t=0 0
a=group:BUNDLE 0 1 2
a=msid-semantic: WMS
m=audio 9 UDP/TLS/RTP/SAVPF 111 103 104 9 102 0 8 106 105 13 110 112 113 126
c=IN IP4 0.0.0.0
a=rtcp:9 IN IP4 0.0.0.0
a=ice-ufrag:nEKv
a=ice-pwd:gqENKe3M0JNAckL3f8j2XiuB
a=ice-options:trickle
a=fingerprint:sha-256 D4:46:16:65:73:28:57:AD:19:52:F3:3A:1B:F1:EA:30:8D:03:AB:F3:C8:B1:55:5B:46:AF:3B:9D:E4:4E:8D:86
a=setup:actpass
a=mid:0
a=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level
a=extmap:9 urn:ietf:params:rtp-hdrext:sdes:mid
a=sendrecv
a=msid:- 087a631f-c024-4876-bccd-1cb65ffbdfb1
a=rtcp-mux
a=rtpmap:111 opus/48000/2
a=rtcp-fb:111 transport-cc
a=fmtp:111 minptime=10;useinbandfec=1
a=rtpmap:103 ISAC/16000
a=rtpmap:104 ISAC/32000
a=rtpmap:9 G722/8000
a=rtpmap:102 ILBC/8000
a=rtpmap:0 PCMU/8000
a=rtpmap:8 PCMA/8000
a=rtpmap:106 CN/32000
a=rtpmap:105 CN/16000
a=rtpmap:13 CN/8000
a=rtpmap:110 telephone-event/48000
a=rtpmap:112 telephone-event/32000
a=rtpmap:113 telephone-event/16000
a=rtpmap:126 telephone-event/8000
a=ssrc:4042814656 cname:xUObbn1tOTJx8jAj
a=ssrc:4042814656 msid: 087a631f-c024-4876-bccd-1cb65ffbdfb1
a=ssrc:4042814656 mslabel:
a=ssrc:4042814656 label:087a631f-c024-4876-bccd-1cb65ffbdfb1
m=video 9 UDP/TLS/RTP/SAVPF 96 97 98 99 100 101 127 124 125
c=IN IP4 0.0.0.0
a=rtcp:9 IN IP4 0.0.0.0
a=ice-ufrag:nEKv
a=ice-pwd:gqENKe3M0JNAckL3f8j2XiuB
a=ice-options:trickle
a=fingerprint:sha-256 D4:46:16:65:73:28:57:AD:19:52:F3:3A:1B:F1:EA:30:8D:03:AB:F3:C8:B1:55:5B:46:AF:3B:9D:E4:4E:8D:86
a=setup:actpass
a=mid:1
a=extmap:2 urn:ietf:params:rtp-hdrext:toffset
a=extmap:3 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time
a=extmap:4 urn:3gpp:video-orientation
a=extmap:5 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01
a=extmap:6 http://www.webrtc.org/experiments/rtp-hdrext/playout-delay
a=extmap:7 http://www.webrtc.org/experiments/rtp-hdrext/video-content-type
a=extmap:8 http://www.webrtc.org/experiments/rtp-hdrext/video-timing
a=extmap:10 http://tools.ietf.org/html/draft-ietf-avtext-framemarking-07
a=extmap:9 urn:ietf:params:rtp-hdrext:sdes:mid
a=sendrecv
a=msid:- 9ac4d69d-1d99-40c7-a5d7-c487d6cf3c91
a=rtcp-mux
a=rtcp-rsize
a=rtpmap:96 VP8/90000
a=rtcp-fb:96 goog-remb
a=rtcp-fb:96 transport-cc
a=rtcp-fb:96 ccm fir
a=rtcp-fb:96 nack
a=rtcp-fb:96 nack pli
a=rtpmap:97 rtx/90000
a=fmtp:97 apt=96
a=rtpmap:98 VP9/90000
a=rtcp-fb:98 goog-remb
a=rtcp-fb:98 transport-cc
a=rtcp-fb:98 ccm fir
a=rtcp-fb:98 nack
a=rtcp-fb:98 nack pli
a=fmtp:98 x-google-profile-id=0
a=rtpmap:99 rtx/90000
a=fmtp:99 apt=98
a=rtpmap:100 multiplex/90000
a=rtcp-fb:100 goog-remb
a=rtcp-fb:100 transport-cc
a=rtcp-fb:100 ccm fir
a=rtcp-fb:100 nack
a=rtcp-fb:100 nack pli
a=fmtp:100 acn=VP9;x-google-profile-id=0
a=rtpmap:101 rtx/90000
a=fmtp:101 apt=100
a=rtpmap:127 red/90000
a=rtpmap:124 rtx/90000
a=fmtp:124 apt=127
a=rtpmap:125 ulpfec/90000
a=ssrc-group:FID 495734452 1701376822
a=ssrc:495734452 cname:xUObbn1tOTJx8jAj
a=ssrc:495734452 msid: 9ac4d69d-1d99-40c7-a5d7-c487d6cf3c91
a=ssrc:495734452 mslabel:
a=ssrc:495734452 label:9ac4d69d-1d99-40c7-a5d7-c487d6cf3c91
a=ssrc:1701376822 cname:xUObbn1tOTJx8jAj
a=ssrc:1701376822 msid: 9ac4d69d-1d99-40c7-a5d7-c487d6cf3c91
a=ssrc:1701376822 mslabel:
a=ssrc:1701376822 label:9ac4d69d-1d99-40c7-a5d7-c487d6cf3c91
m=application 9 DTLS/SCTP 5000
c=IN IP4 0.0.0.0
a=ice-ufrag:nEKv
a=ice-pwd:gqENKe3M0JNAckL3f8j2XiuB
a=ice-options:trickle
a=fingerprint:sha-256 D4:46:16:65:73:28:57:AD:19:52:F3:3A:1B:F1:EA:30:8D:03:AB:F3:C8:B1:55:5B:46:AF:3B:9D:E4:4E:8D:86
a=setup:actpass
a=mid:2
a=sctpmap:5000 webrtc-datachannel 1024
)";

static std::string subSdp = R"(
v=0
o=- 348156985326963886 2 IN IP4 127.0.0.1
s=-
t=0 0
a=group:BUNDLE 0 1 2
a=msid-semantic: WMS
m=audio 9 UDP/TLS/RTP/SAVPF 111 103 104 9 102 0 8 106 105 13 110 112 113 126
c=IN IP4 0.0.0.0
a=rtcp:9 IN IP4 0.0.0.0
a=ice-ufrag:H23a
a=ice-pwd:5WQZdpjHkA/bIkd+bNf/3kfJ
a=ice-options:trickle
a=fingerprint:sha-256 1C:A5:4D:94:7B:3D:10:99:43:3F:8D:B4:15:01:75:F6:1F:C8:27:88:8A:0A:5E:EE:50:55:FC:BC:0A:13:25:E2
a=setup:active
a=mid:0
a=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level
a=extmap:9 urn:ietf:params:rtp-hdrext:sdes:mid
a=sendrecv
a=msid:- b3bc385d-c1bf-418a-b125-1ac6d1564c36
a=rtcp-mux
a=rtpmap:111 opus/48000/2
a=rtcp-fb:111 transport-cc
a=fmtp:111 minptime=10;useinbandfec=1
a=rtpmap:103 ISAC/16000
a=rtpmap:104 ISAC/32000
a=rtpmap:9 G722/8000
a=rtpmap:102 ILBC/8000
a=rtpmap:0 PCMU/8000
a=rtpmap:8 PCMA/8000
a=rtpmap:106 CN/32000
a=rtpmap:105 CN/16000
a=rtpmap:13 CN/8000
a=rtpmap:110 telephone-event/48000
a=rtpmap:112 telephone-event/32000
a=rtpmap:113 telephone-event/16000
a=rtpmap:126 telephone-event/8000
a=ssrc:865236202 cname:wEBkzPwRTpmzaNTY
m=video 9 UDP/TLS/RTP/SAVPF 96 97 98 99 100 101 127 124 125
c=IN IP4 0.0.0.0
a=rtcp:9 IN IP4 0.0.0.0
a=ice-ufrag:H23a
a=ice-pwd:5WQZdpjHkA/bIkd+bNf/3kfJ
a=ice-options:trickle
a=fingerprint:sha-256 1C:A5:4D:94:7B:3D:10:99:43:3F:8D:B4:15:01:75:F6:1F:C8:27:88:8A:0A:5E:EE:50:55:FC:BC:0A:13:25:E2
a=setup:active
a=mid:1
a=extmap:2 urn:ietf:params:rtp-hdrext:toffset
a=extmap:3 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time
a=extmap:4 urn:3gpp:video-orientation
a=extmap:5 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01
a=extmap:6 http://www.webrtc.org/experiments/rtp-hdrext/playout-delay
a=extmap:7 http://www.webrtc.org/experiments/rtp-hdrext/video-content-type
a=extmap:8 http://www.webrtc.org/experiments/rtp-hdrext/video-timing
a=extmap:10 http://tools.ietf.org/html/draft-ietf-avtext-framemarking-07
a=extmap:9 urn:ietf:params:rtp-hdrext:sdes:mid
a=sendrecv
a=msid:- 0e1c8f8e-4c17-43b1-b5dd-ff1f426ae147
a=rtcp-mux
a=rtcp-rsize
a=rtpmap:96 VP8/90000
a=rtcp-fb:96 goog-remb
a=rtcp-fb:96 transport-cc
a=rtcp-fb:96 ccm fir
a=rtcp-fb:96 nack
a=rtcp-fb:96 nack pli
a=rtpmap:97 rtx/90000
a=fmtp:97 apt=96
a=rtpmap:98 VP9/90000
a=rtcp-fb:98 goog-remb
a=rtcp-fb:98 transport-cc
a=rtcp-fb:98 ccm fir
a=rtcp-fb:98 nack
a=rtcp-fb:98 nack pli
a=fmtp:98 x-google-profile-id=0
a=rtpmap:99 rtx/90000
a=fmtp:99 apt=98
a=rtpmap:100 multiplex/90000
a=rtcp-fb:100 goog-remb
a=rtcp-fb:100 transport-cc
a=rtcp-fb:100 ccm fir
a=rtcp-fb:100 nack
a=rtcp-fb:100 nack pli
a=fmtp:100 acn=VP9;x-google-profile-id=0
a=rtpmap:101 rtx/90000
a=fmtp:101 apt=100
a=rtpmap:127 red/90000
a=rtpmap:124 rtx/90000
a=fmtp:124 apt=127
a=rtpmap:125 ulpfec/90000
a=ssrc-group:FID 1527860554 664206513
a=ssrc:1527860554 cname:wEBkzPwRTpmzaNTY
a=ssrc:664206513 cname:wEBkzPwRTpmzaNTY
m=application 9 DTLS/SCTP 5000
c=IN IP4 0.0.0.0
b=AS:30
a=ice-ufrag:H23a
a=ice-pwd:5WQZdpjHkA/bIkd+bNf/3kfJ
a=ice-options:trickle
a=fingerprint:sha-256 1C:A5:4D:94:7B:3D:10:99:43:3F:8D:B4:15:01:75:F6:1F:C8:27:88:8A:0A:5E:EE:50:55:FC:BC:0A:13:25:E2
a=setup:active
a=mid:2
a=sctpmap:5000 webrtc-datachannel 1024
)";

#include "Master/sdptransform.hpp"

void test()
{
    json sdp;
    sdp["sdp"] = egSdp;
    printf("%s", sdp.dump().c_str());
}

void testSdp()
{
    //auto jsonSdp = sdptransform::parse(pubOfferSdp);
    auto jsonSdp = sdptransform::parse(egSdp);
    spdlog::get(PMS_LOGGER)->info("pub sdp \n{}", jsonSdp.dump());
//    SdpInfo sdp;
}

void testTransform()
{
    pingos::SdpInfo si(egSdp);
    pingos::WebRtcTransportParameters rtcTransportParameters;
    std::vector<pingos::ProducerParameters> producerParameters;

    si.TransformSdp(rtcTransportParameters, producerParameters);

    rtcTransportParameters.AddLocalAddr("127.0.0.1", "0.0.0.0");

    json jsonTransport, jsonDtls;
    rtcTransportParameters.FillJsonTransport(jsonTransport);
    rtcTransportParameters.FillJsonDtls(jsonDtls);

    spdlog::get(PMS_LOGGER)->info("transport \n{} \n dtls \n {}",
        jsonTransport.dump(), jsonDtls.dump());

    for (auto &producer : producerParameters) {
        json jsonObject;
        producer.FillJson(jsonObject);
        spdlog::get(PMS_LOGGER)->info("producer \n {}",
            jsonObject.dump());
        pingos::ConsumerParameters consumer;
        consumer.SetRtpParameters(producer);
        consumer.FillJson(jsonObject);
        spdlog::get(PMS_LOGGER)->info("consumer \n {}",
            jsonObject.dump());
    }
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

    pingos::WssServer ns;
    if (pingos::Configuration::websocket.ssl) {
        ns.Accept(pingos::Configuration::websocket.port,
            pingos::Configuration::websocket.keyFile,
            pingos::Configuration::websocket.certFile,
            pingos::Configuration::websocket.passPhrase);
    } else {
        ns.Accept(pingos::Configuration::websocket.port);
    }

    pingos::RtcMaster master;

    pingos::RtcServer rtc(&ns, &master);

    pingos::Master::Options opt = {
        .execDir = pingos::Configuration::master.execPath,
        .nWorkers = pingos::Configuration::master.numOfWorkerProcess,
        .daemon = false
    };

    master.StartWorkers(opt);

//    testSdp();
//    testTransform();

    pingos::Loop::Run();
}
