```json
"rtpParameters":{
                "mid":"0",
                "codecs":[
                    {
                        "mimeType":"audio/opus",
                        "clockRate":48000,
                        "payloadType":111,
                        "channels":2,
                        "rtcpFeedback":[
                            {
                                "type":"transport-cc"
                            }
                        ],
                        "parameters":{
                            "minptime":10,
                            "useinbandfec":1,
                            "sprop-stereo":1,
                            "usedtx":1
                        }
                    }
                ],
                "headerExtensions":[
                    {
                        "uri":"urn:ietf:params:rtp-hdrext:sdes:mid",
                        "id":4
                    },
                    {
                        "uri":"http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time",
                        "id":2
                    },
                    {
                        "uri":"http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01",
                        "id":3
                    },
                    {
                        "uri":"urn:ietf:params:rtp-hdrext:ssrc-audio-level",
                        "id":1
                    }
                ],
                "encodings":[
                    {
                        "ssrc":3355957794
                    }
                ],
                "rtcp":{
                    "cname":"TdifV0DCOPRR7RAD"
                }
            },
            "rtpMapping":{
                "codecs":[
                    {
                        "payloadType":111,
                        "mappedPayloadType":100
                    }
                ],
                "encodings":[
                    {
                        "mappedSsrc":994535925,
                        "ssrc":3355957794
                    }
                ]
            },
```
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