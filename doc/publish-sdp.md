# publish offer

## sdp

```sdp
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
```

## json

```json
{
    "groups":[
        {
            "mids":"0 1 2",
            "type":"BUNDLE"
        }
    ],
    "media":[
        {
            "connection":{
                "ip":"0.0.0.0",
                "version":4
            },
            "direction":"sendrecv",
            "ext":[
                {
                    "uri":"urn:ietf:params:rtp-hdrext:ssrc-audio-level",
                    "value":1
                },
                {
                    "uri":"urn:ietf:params:rtp-hdrext:sdes:mid",
                    "value":9
                }
            ],
            "fingerprint":{
                "hash":"D4:46:16:65:73:28:57:AD:19:52:F3:3A:1B:F1:EA:30:8D:03:AB:F3:C8:B1:55:5B:46:AF:3B:9D:E4:4E:8D:86",
                "type":"sha-256"
            },
            "fmtp":[
                {
                    "config":"minptime=10;useinbandfec=1",
                    "payload":111
                }
            ],
            "iceOptions":"trickle",
            "icePwd":"gqENKe3M0JNAckL3f8j2XiuB",
            "iceUfrag":"nEKv",
            "mid":"0",
            "msid":"- 087a631f-c024-4876-bccd-1cb65ffbdfb1",
            "payloads":"111 103 104 9 102 0 8 106 105 13 110 112 113 126",
            "port":9,
            "protocol":"UDP/TLS/RTP/SAVPF",
            "rtcp":{
                "address":"0.0.0.0",
                "ipVer":4,
                "netType":"IN",
                "port":9
            },
            "rtcpFb":[
                {
                    "payload":"111",
                    "type":"transport-cc"
                }
            ],
            "rtcpMux":"rtcp-mux",
            "rtp":[
                {
                    "codec":"opus",
                    "encoding":"2",
                    "payload":111,
                    "rate":48000
                },
                {
                    "codec":"ISAC",
                    "payload":103,
                    "rate":16000
                },
                {
                    "codec":"ISAC",
                    "payload":104,
                    "rate":32000
                },
                {
                    "codec":"G722",
                    "payload":9,
                    "rate":8000
                },
                {
                    "codec":"ILBC",
                    "payload":102,
                    "rate":8000
                },
                {
                    "codec":"PCMU",
                    "payload":0,
                    "rate":8000
                },
                {
                    "codec":"PCMA",
                    "payload":8,
                    "rate":8000
                },
                {
                    "codec":"CN",
                    "payload":106,
                    "rate":32000
                },
                {
                    "codec":"CN",
                    "payload":105,
                    "rate":16000
                },
                {
                    "codec":"CN",
                    "payload":13,
                    "rate":8000
                },
                {
                    "codec":"telephone-event",
                    "payload":110,
                    "rate":48000
                },
                {
                    "codec":"telephone-event",
                    "payload":112,
                    "rate":32000
                },
                {
                    "codec":"telephone-event",
                    "payload":113,
                    "rate":16000
                },
                {
                    "codec":"telephone-event",
                    "payload":126,
                    "rate":8000
                }
            ],
            "setup":"actpass",
            "ssrcs":[
                {
                    "attribute":"cname",
                    "id":4042814656,
                    "value":"xUObbn1tOTJx8jAj"
                },
                {
                    "attribute":"msid",
                    "id":4042814656,
                    "value":" 087a631f-c024-4876-bccd-1cb65ffbdfb1"
                },
                {
                    "attribute":"mslabel",
                    "id":4042814656
                },
                {
                    "attribute":"label",
                    "id":4042814656,
                    "value":"087a631f-c024-4876-bccd-1cb65ffbdfb1"
                }
            ],
            "type":"audio"
        },
        {
            "connection":{
                "ip":"0.0.0.0",
                "version":4
            },
            "direction":"sendrecv",
            "ext":[
                {
                    "uri":"urn:ietf:params:rtp-hdrext:toffset",
                    "value":2
                },
                {
                    "uri":"http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time",
                    "value":3
                },
                {
                    "uri":"urn:3gpp:video-orientation",
                    "value":4
                },
                {
                    "uri":"http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01",
                    "value":5
                },
                {
                    "uri":"http://www.webrtc.org/experiments/rtp-hdrext/playout-delay",
                    "value":6
                },
                {
                    "uri":"http://www.webrtc.org/experiments/rtp-hdrext/video-content-type",
                    "value":7
                },
                {
                    "uri":"http://www.webrtc.org/experiments/rtp-hdrext/video-timing",
                    "value":8
                },
                {
                    "uri":"http://tools.ietf.org/html/draft-ietf-avtext-framemarking-07",
                    "value":10
                },
                {
                    "uri":"urn:ietf:params:rtp-hdrext:sdes:mid",
                    "value":9
                }
            ],
            "fingerprint":{
                "hash":"D4:46:16:65:73:28:57:AD:19:52:F3:3A:1B:F1:EA:30:8D:03:AB:F3:C8:B1:55:5B:46:AF:3B:9D:E4:4E:8D:86",
                "type":"sha-256"
            },
            "fmtp":[
                {
                    "config":"apt=96",
                    "payload":97
                },
                {
                    "config":"x-google-profile-id=0",
                    "payload":98
                },
                {
                    "config":"apt=98",
                    "payload":99
                },
                {
                    "config":"acn=VP9;x-google-profile-id=0",
                    "payload":100
                },
                {
                    "config":"apt=100",
                    "payload":101
                },
                {
                    "config":"apt=127",
                    "payload":124
                }
            ],
            "iceOptions":"trickle",
            "icePwd":"gqENKe3M0JNAckL3f8j2XiuB",
            "iceUfrag":"nEKv",
            "mid":"1",
            "msid":"- 9ac4d69d-1d99-40c7-a5d7-c487d6cf3c91",
            "payloads":"96 97 98 99 100 101 127 124 125",
            "port":9,
            "protocol":"UDP/TLS/RTP/SAVPF",
            "rtcp":{
                "address":"0.0.0.0",
                "ipVer":4,
                "netType":"IN",
                "port":9
            },
            "rtcpFb":[
                {
                    "payload":"96",
                    "type":"goog-remb"
                },
                {
                    "payload":"96",
                    "type":"transport-cc"
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
                },
                {
                    "payload":"98",
                    "type":"goog-remb"
                },
                {
                    "payload":"98",
                    "type":"transport-cc"
                },
                {
                    "payload":"98",
                    "subtype":"fir",
                    "type":"ccm"
                },
                {
                    "payload":"98",
                    "type":"nack"
                },
                {
                    "payload":"98",
                    "subtype":"pli",
                    "type":"nack"
                },
                {
                    "payload":"100",
                    "type":"goog-remb"
                },
                {
                    "payload":"100",
                    "type":"transport-cc"
                },
                {
                    "payload":"100",
                    "subtype":"fir",
                    "type":"ccm"
                },
                {
                    "payload":"100",
                    "type":"nack"
                },
                {
                    "payload":"100",
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
                },
                {
                    "codec":"VP9",
                    "payload":98,
                    "rate":90000
                },
                {
                    "codec":"rtx",
                    "payload":99,
                    "rate":90000
                },
                {
                    "codec":"multiplex",
                    "payload":100,
                    "rate":90000
                },
                {
                    "codec":"rtx",
                    "payload":101,
                    "rate":90000
                },
                {
                    "codec":"red",
                    "payload":127,
                    "rate":90000
                },
                {
                    "codec":"rtx",
                    "payload":124,
                    "rate":90000
                },
                {
                    "codec":"ulpfec",
                    "payload":125,
                    "rate":90000
                }
            ],
            "setup":"actpass",
            "ssrcGroups":[
                {
                    "semantics":"FID",
                    "ssrcs":"495734452 1701376822"
                }
            ],
            "ssrcs":[
                {
                    "attribute":"cname",
                    "id":495734452,
                    "value":"xUObbn1tOTJx8jAj"
                },
                {
                    "attribute":"msid",
                    "id":495734452,
                    "value":" 9ac4d69d-1d99-40c7-a5d7-c487d6cf3c91"
                },
                {
                    "attribute":"mslabel",
                    "id":495734452
                },
                {
                    "attribute":"label",
                    "id":495734452,
                    "value":"9ac4d69d-1d99-40c7-a5d7-c487d6cf3c91"
                },
                {
                    "attribute":"cname",
                    "id":1701376822,
                    "value":"xUObbn1tOTJx8jAj"
                },
                {
                    "attribute":"msid",
                    "id":1701376822,
                    "value":" 9ac4d69d-1d99-40c7-a5d7-c487d6cf3c91"
                },
                {
                    "attribute":"mslabel",
                    "id":1701376822
                },
                {
                    "attribute":"label",
                    "id":1701376822,
                    "value":"9ac4d69d-1d99-40c7-a5d7-c487d6cf3c91"
                }
            ],
            "type":"video"
        },
        {
            "connection":{
                "ip":"0.0.0.0",
                "version":4
            },
            "fingerprint":{
                "hash":"D4:46:16:65:73:28:57:AD:19:52:F3:3A:1B:F1:EA:30:8D:03:AB:F3:C8:B1:55:5B:46:AF:3B:9D:E4:4E:8D:86",
                "type":"sha-256"
            },
            "fmtp":[

            ],
            "iceOptions":"trickle",
            "icePwd":"gqENKe3M0JNAckL3f8j2XiuB",
            "iceUfrag":"nEKv",
            "mid":"2",
            "payloads":"5000",
            "port":9,
            "protocol":"DTLS/SCTP",
            "rtp":[

            ],
            "sctpmap":{
                "app":"webrtc-datachannel",
                "maxMessageSize":1024,
                "sctpmapNumber":5000
            },
            "setup":"actpass",
            "type":"application"
        }
    ],
    "msidSemantic":{
        "token":"WMS"
    },
    "name":"-",
    "origin":{
        "address":"127.0.0.1",
        "ipVer":4,
        "netType":"IN",
        "sessionId":235914367549117899,
        "sessionVersion":2,
        "username":"-"
    },
    "timing":{
        "start":0,
        "stop":0
    },
    "version":0
}
```

# publish ack sdp

## sdp

```sdp
v=0
o=pingos-client 1603013435498732369 2 IN IP4 0.0.0.0
s=-
c=IN IP4 127.0.0.1
t=0 0
a=ice-lite
a=msid-semantic: WMS *
a=group:BUNDLE audio video
m=audio 9 RTP/SAVPF 111
a=rtpmap:111 opus/48000/2
a=fmtp:111 minptime=10;useinbandfec=1
a=rtcp-fb:111 nack
a=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level
a=setup:active
a=mid:audio
a=sendrecv
a=ice-ufrag:f97yz8h0o871875q
a=ice-pwd:8yon4lippdcqdwyrx710lun6imxfy9zw
a=fingerprint:sha-256 ED:BB:50:26:F1:66:50:09:59:AC:8A:E7:81:F0:8C:7A:AE:1B:B4:42:80:DA:5F:DB:80:05:0B:2A:89:36:B9:6F
a=candidate:udpcandidate 1 udp 1078862079 106.75.167.159 26027 typ host
a=candidate:tcpcandidate 1 tcp 1078862079 106.75.167.159 29037 typ host
a=rtcp-mux
a=rtcp-rsize
m=video 9 RTP/SAVPF 96 97
b=AS:600
a=rtpmap:96 VP8/90000
a=rtpmap:97 rtx/90000
a=fmtp:97 apt=96
a=fmtp:97 x-google-min-bitrate=200
a=x-google-flag:conference
a=rtcp-fb:96 goog-remb
a=rtcp-fb:96 ccm fir
a=rtcp-fb:96 nack
a=rtcp-fb:96 nack pli
a=extmap:5 http://www.webrtc.org/experiments/rtp-hdrext/playout-delay
a=extmap:6 http://www.webrtc.org/experiments/rtp-hdrext/video-content-type
a=extmap:7 http://www.webrtc.org/experiments/rtp-hdrext/video-timing
a=extmap:8 http://www.webrtc.org/experiments/rtp-hdrext/color-space
a=extmap:14 urn:ietf:params:rtp-hdrext:toffset
a=extmap:2 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time
a=extmap:13 urn:3gpp:video-orientation
a=setup:active
a=mid:video
a=sendrecv
a=ice-ufrag:f97yz8h0o871875q
a=ice-pwd:8yon4lippdcqdwyrx710lun6imxfy9zw
a=fingerprint:sha-256 ED:BB:50:26:F1:66:50:09:59:AC:8A:E7:81:F0:8C:7A:AE:1B:B4:42:80:DA:5F:DB:80:05:0B:2A:89:36:B9:6F
a=candidate:udpcandidate 1 udp 1078862079 106.75.167.159 26027 typ host
a=candidate:tcpcandidate 1 tcp 1078862079 106.75.167.159 29037 typ host
a=rtcp-mux
a=rtcp-rsize
```

## json

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
                    "port":26027,
                    "priority":1078862079,
                    "transport":"udp",
                    "type":"host"
                },
                {
                    "component":1,
                    "foundation":"tcpcandidate",
                    "ip":"106.75.167.159",
                    "port":29037,
                    "priority":1078862079,
                    "transport":"tcp",
                    "type":"host"
                }
            ],
            "direction":"sendrecv",
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
            "icePwd":"8yon4lippdcqdwyrx710lun6imxfy9zw",
            "iceUfrag":"f97yz8h0o871875q",
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
            "type":"audio"
        },
        {
            "bandwidth":[
                {
                    "limit":600,
                    "type":"AS"
                }
            ],
            "candidates":[
                {
                    "component":1,
                    "foundation":"udpcandidate",
                    "ip":"106.75.167.159",
                    "port":26027,
                    "priority":1078862079,
                    "transport":"udp",
                    "type":"host"
                },
                {
                    "component":1,
                    "foundation":"tcpcandidate",
                    "ip":"106.75.167.159",
                    "port":29037,
                    "priority":1078862079,
                    "transport":"tcp",
                    "type":"host"
                }
            ],
            "direction":"sendrecv",
            "ext":[
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
                },
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
                    "config":"x-google-min-bitrate=200",
                    "payload":97
                }
            ],
            "icePwd":"8yon4lippdcqdwyrx710lun6imxfy9zw",
            "iceUfrag":"f97yz8h0o871875q",
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
            "type":"video",
            "xGoogleFlag":"conference"
        }
    ],
    "msidSemantic":{
        "semantic":"WMS",
        "token":"*"
    },
    "name":"-",
    "origin":{
        "address":"0.0.0.0",
        "ipVer":4,
        "netType":"IN",
        "sessionId":1603013435498732369,
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
