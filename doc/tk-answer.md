v=0
o=- 4147213245290703667 2 IN IP4 127.0.0.1
s=-
t=0 0
a=group:BUNDLE 0 1 2
a=msid-semantic: WMS
m=audio 9 UDP/TLS/RTP/SAVPF 111 103 104 9 102 0 8 106 105 13 110 112 113 126
c=IN IP4 0.0.0.0
a=rtcp:9 IN IP4 0.0.0.0
a=ice-ufrag:qseO
a=ice-pwd:vKdBamgzOKIF5QxHS/aTf/RM
a=ice-options:trickle
a=fingerprint:sha-256 6D:DF:EE:35:07:3F:CE:42:49:97:99:52:68:FE:AF:B7:16:65:29:AA:2F:CF:05:95:08:18:5C:E6:CB:88:0A:D4
a=setup:active
a=mid:0
a=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level
a=extmap:9 urn:ietf:params:rtp-hdrext:sdes:mid
a=sendrecv
a=msid:- c975ef9c-2fed-45b0-b123-b06a227f96a8
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
a=ssrc:2158495825 cname:C5WLqz8rD/US/yHx
m=video 9 UDP/TLS/RTP/SAVPF 96 97 98 99 100 101 127 124 125
c=IN IP4 0.0.0.0
a=rtcp:9 IN IP4 0.0.0.0
a=ice-ufrag:qseO
a=ice-pwd:vKdBamgzOKIF5QxHS/aTf/RM
a=ice-options:trickle
a=fingerprint:sha-256 6D:DF:EE:35:07:3F:CE:42:49:97:99:52:68:FE:AF:B7:16:65:29:AA:2F:CF:05:95:08:18:5C:E6:CB:88:0A:D4
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
a=msid:- 137c5d43-3efa-4fc5-869e-c50faf268a75
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
a=ssrc-group:FID 3143957944 4235249497
a=ssrc:3143957944 cname:C5WLqz8rD/US/yHx
a=ssrc:4235249497 cname:C5WLqz8rD/US/yHx
m=application 9 DTLS/SCTP 5000
c=IN IP4 0.0.0.0
b=AS:30
a=ice-ufrag:qseO
a=ice-pwd:vKdBamgzOKIF5QxHS/aTf/RM
a=ice-options:trickle
a=fingerprint:sha-256 6D:DF:EE:35:07:3F:CE:42:49:97:99:52:68:FE:AF:B7:16:65:29:AA:2F:CF:05:95:08:18:5C:E6:CB:88:0A:D4
a=setup:active
a=mid:2
a=sctpmap:5000 webrtc-datachannel 1024