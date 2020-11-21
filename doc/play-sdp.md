```sdp
v=0
o=- 5920129321270894351 2 IN IP4 127.0.0.1
s=-
t=0 0
a=group:BUNDLE audio video
a=msid-semantic: WMS
m=audio 9 UDP/TLS/RTP/SAVPF 111 103 104 9 0 8 106 105 13 110 112 113 126
c=IN IP4 0.0.0.0
a=rtcp:9 IN IP4 0.0.0.0
a=ice-ufrag:rzoY
a=ice-pwd:gnGYEuPKt78l85OH+aAcZ6ZG
a=ice-options:trickle
a=fingerprint:sha-256 1D:48:24:5B:9A:F1:8E:11:B6:B0:B9:83:31:D6:A3:B7:5A:7B:32:81:7C:94:37:31:5E:14:5C:2D:AC:CE:4A:4C
a=setup:actpass
a=mid:audio
a=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level
a=extmap:2 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time
a\=extmap:3 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01
a\=recvonly
a\=rtcp-mux
a\=rtpmap:111 opus/48000/2
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
m=video 9 UDP/TLS/RTP/SAVPF 96 97 98 99 100 101 122 102 121 127 120 125 107 108 109 124 119 123 118 114 115 116
c=IN IP4 0.0.0.0
a=rtcp:9 IN IP4 0.0.0.0
a=ice-ufrag:rzoY
a=ice-pwd:gnGYEuPKt78l85OH+aAcZ6ZG
a=ice-options:trickle
a=fingerprint:sha-256 1D:48:24:5B:9A:F1:8E:11:B6:B0:B9:83:31:D6:A3:B7:5A:7B:32:81:7C:94:37:31:5E:14:5C:2D:AC:CE:4A:4C
a=setup:actpass
a=mid:video
a=extmap:14 urn:ietf:params:rtp-hdrext:toffset
a=extmap:2 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time
a\=extmap:13 urn:3gpp:video-orientation
a=extmap:3 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01
a\=extmap:5 http://www.webrtc.org/experiments/rtp-hdrext/playout-delay
a\=extmap:6 http://www.webrtc.org/experiments/rtp-hdrext/video-content-type
a\=extmap:7 http://www.webrtc.org/experiments/rtp-hdrext/video-timing
a\=extmap:8 http://www.webrtc.org/experiments/rtp-hdrext/color-space
a\=recvonly
a\=rtcp-mux
a\=rtcp-rsize
a\=rtpmap:96 VP8/90000
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
a=rtpmap:122 VP9/90000
a=rtcp-fb:122 goog-remb
a=rtcp-fb:122 ccm fir
a=rtcp-fb:122 nack
a=rtcp-fb:122 nack pli
a=fmtp:122 profile-id=1
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
```

```sdp
v=0
o=pingos-client 1603159863350814327 2 IN IP4 0.0.0.0
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
```

```json
{
    "connection":{
        "ip":"127.0.0.1",
        "version":4
    },
    "groups":[
        {
            "mids":"audio video",
            "type":"BUNDLE"
        }
    ],
    "icelite":"ice-lite",
    "media":[
        {
            "candidates":[
                {
                    "component":1,
                    "foundation":"udpcandidate",
                    "ip":"106.75.167.159",
                    "port":26291,
                    "priority":1078862079,
                    "transport":"udp",
                    "type":"host"
                },
                {
                    "component":1,
                    "foundation":"tcpcandidate",
                    "ip":"106.75.167.159",
                    "port":29364,
                    "priority":1078862079,
                    "transport":"tcp",
                    "type":"host"
                }
            ],
            "direction":"sendonly",
            "ext":[
                {
                    "uri":"urn:ietf:params:rtp-hdrext:ssrc-audio-level",
                    "value":1
                }
            ],
            "fingerprint":{
                "hash":"ED:BB:50:26:F1:66:50:09:59:AC:8A:E7:81:F0:8C:7A:AE:1B:B4:42:80:DA:5F:DB:80:05:0B:2A:89:36:B9:6F",
                "type":"sha-256"
            },
            "fmtp":[
                {
                    "config":"minptime=10;useinbandfec=1",
                    "payload":111
                }
            ],
            "icePwd":"cpsgcf54fd6ce4jc1uxd1gihkabhflst",
            "iceUfrag":"zcbnnmkjyw9rhnmj",
            "mid":"audio",
            "payloads":"111",
            "port":9,
            "protocol":"RTP/SAVPF",
            "rtcpFb":[
                {
                    "payload":"111",
                    "type":"nack"
                }
            ],
            "rtcpMux":"rtcp-mux",
            "rtcpRsize":"rtcp-rsize",
            "rtp":[
                {
                    "codec":"opus",
                    "encoding":"2",
                    "payload":111,
                    "rate":48000
                }
            ],
            "setup":"active",
            "ssrcs":[
                {
                    "attribute":"cname",
                    "id":843551455,
                    "value":"hcBJWbcohEvJDMRv"
                },
                {
                    "attribute":"msid",
                    "id":843551455,
                    "value":"d0026a4ba79f0a12bc7c07Y24tZ2Q1093463830VP8_1-3126898112 d0026a4ba79f0a12bc7c07Y24tZ2Q1093463830VP8_1-3126898112_audio"
                },
                {
                    "attribute":"mslabel",
                    "id":843551455,
                    "value":"d0026a4ba79f0a12bc7c07Y24tZ2Q1093463830VP8_1-3126898112"
                },
                {
                    "attribute":"label",
                    "id":843551455,
                    "value":"d0026a4ba79f0a12bc7c07Y24tZ2Q1093463830VP8_1-3126898112_audio"
                }
            ],
            "type":"audio"
        },
        {
            "bandwidth":[
                {
                    "limit":800,
                    "type":"AS"
                }
            ],
            "candidates":[
                {
                    "component":1,
                    "foundation":"udpcandidate",
                    "ip":"106.75.167.159",
                    "port":26291,
                    "priority":1078862079,
                    "transport":"udp",
                    "type":"host"
                },
                {
                    "component":1,
                    "foundation":"tcpcandidate",
                    "ip":"106.75.167.159",
                    "port":29364,
                    "priority":1078862079,
                    "transport":"tcp",
                    "type":"host"
                }
            ],
            "direction":"sendonly",
            "ext":[
                {
                    "uri":"http://www.webrtc.org/experiments/rtp-hdrext/color-space",
                    "value":8
                },
                {
                    "uri":"urn:ietf:params:rtp-hdrext:toffset",
                    "value":14
                },
                {
                    "uri":"http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time",
                    "value":2
                },
                {
                    "uri":"urn:3gpp:video-orientation",
                    "value":13
                },
                {
                    "uri":"http://www.webrtc.org/experiments/rtp-hdrext/playout-delay",
                    "value":5
                },
                {
                    "uri":"http://www.webrtc.org/experiments/rtp-hdrext/video-content-type",
                    "value":6
                },
                {
                    "uri":"http://www.webrtc.org/experiments/rtp-hdrext/video-timing",
                    "value":7
                }
            ],
            "fingerprint":{
                "hash":"ED:BB:50:26:F1:66:50:09:59:AC:8A:E7:81:F0:8C:7A:AE:1B:B4:42:80:DA:5F:DB:80:05:0B:2A:89:36:B9:6F",
                "type":"sha-256"
            },
            "fmtp":[
                {
                    "config":"apt=96",
                    "payload":97
                },
                {
                    "config":"x-google-min-bitrate=100",
                    "payload":97
                }
            ],
            "icePwd":"cpsgcf54fd6ce4jc1uxd1gihkabhflst",
            "iceUfrag":"zcbnnmkjyw9rhnmj",
            "mid":"video",
            "payloads":"96 97",
            "port":9,
            "protocol":"RTP/SAVPF",
            "rtcpFb":[
                {
                    "payload":"96",
                    "type":"goog-remb"
                },
                {
                    "payload":"96",
                    "subtype":"fir",
                    "type":"ccm"
                },
                {
                    "payload":"96",
                    "type":"nack"
                },
                {
                    "payload":"96",
                    "subtype":"pli",
                    "type":"nack"
                }
            ],
            "rtcpMux":"rtcp-mux",
            "rtcpRsize":"rtcp-rsize",
            "rtp":[
                {
                    "codec":"VP8",
                    "payload":96,
                    "rate":90000
                },
                {
                    "codec":"rtx",
                    "payload":97,
                    "rate":90000
                }
            ],
            "setup":"active",
            "ssrcGroups":[
                {
                    "semantics":"FID",
                    "ssrcs":"767184541 476234506"
                }
            ],
            "ssrcs":[
                {
                    "attribute":"cname",
                    "id":767184541,
                    "value":"hcBJWbcohEvJDMRv"
                },
                {
                    "attribute":"msid",
                    "id":767184541,
                    "value":"d0026a4ba79f0a12bc7c07Y24tZ2Q1093463830VP8_1-3126898112 d0026a4ba79f0a12bc7c07Y24tZ2Q1093463830VP8_1-3126898112_video"
                },
                {
                    "attribute":"mslabel",
                    "id":767184541,
                    "value":"d0026a4ba79f0a12bc7c07Y24tZ2Q1093463830VP8_1-3126898112"
                },
                {
                    "attribute":"label",
                    "id":767184541,
                    "value":"d0026a4ba79f0a12bc7c07Y24tZ2Q1093463830VP8_1-3126898112_video"
                },
                {
                    "attribute":"cname",
                    "id":476234506,
                    "value":"hcBJWbcohEvJDMRv"
                },
                {
                    "attribute":"msid",
                    "id":476234506,
                    "value":"d0026a4ba79f0a12bc7c07Y24tZ2Q1093463830VP8_1-3126898112 d0026a4ba79f0a12bc7c07Y24tZ2Q1093463830VP8_1-3126898112_video"
                },
                {
                    "attribute":"mslabel",
                    "id":476234506,
                    "value":"d0026a4ba79f0a12bc7c07Y24tZ2Q1093463830VP8_1-3126898112"
                },
                {
                    "attribute":"label",
                    "id":476234506,
                    "value":"d0026a4ba79f0a12bc7c07Y24tZ2Q1093463830VP8_1-3126898112_video"
                }
            ],
            "type":"video",
            "xGoogleFlag":"conference"
        }
    ],
    "msidSemantic":{
        "semantic":"WMS",
        "token":"d0026a4ba79f0a12bc7c07Y24tZ2Q1093463830VP8_1-3126898112"
    },
    "name":"-",
    "origin":{
        "address":"0.0.0.0",
        "ipVer":4,
        "netType":"IN",
        "sessionId":1603159863350814327,
        "sessionVersion":2,
        "username":"pingos-client"
    },
    "timing":{
        "start":0,
        "stop":0
    },
    "version":0
}
```