    'use strict';

    let offerOptions = {
        offerToReceiveAudio: 1,
        offerToSendAudio: 1,
        offerToReceiveVideo: 1,
        offerToSendVideo: 1,
    };

class LocalMedia {
    localSdp;
    localStream;
    videoDisplay;
    peerConnection;

    async open(videoId) {
        try {
            this.videoDisplay = document.getElementById(videoId);

            this.localStream = await navigator.mediaDevices.getUserMedia({
                video:{
                    mandatory:{
                        maxWidth: 640,
                        maxHeight: 360
                    }
                },
                audio:true});
            console.log('Received local stream', this.localStream);
            this.videoDisplay.srcObject = this.localStream;
            this.peerConnection = new RTCPeerConnection({ sdpSemantics: "plan-b" });

            this.peerConnection.addEventListener('iceconnectionstatechange', () => {
                switch (this.peerConnection.iceConnectionState) {
                    case 'checking':
                        console.log('@connectionstatechange, connecting');
                        break;
                    case 'connected':
                    case 'completed':
                        console.log('@connectionstatechange, connected');
                        break;
                    case 'failed':
                        console.log('@connectionstatechange, failed');
                        break;
                    case 'disconnected':
                        console.log('@connectionstatechange, disconnected');
                        break;
                    case 'closed':
                        console.log('@connectionstatechange, closed');
                        break;
                }
            });

            this.localStream.getTracks().forEach(track => this.peerConnection.addTrack(track, this.localStream));

            this.peerConnection.offer = await this.peerConnection.createOffer(offerOptions);
            console.log("createOffer", this.peerConnection.offer);
            await this.peerConnection.setLocalDescription(this.peerConnection.offer);

            this.localSdp = this.peerConnection.offer.sdp;
        } catch (e) {
            console.log(`getPublish error: ${e}`);
        }
    }

    setRemoteSdp(sdp) {
        console.log("set remote sdp: " + sdp);
        let answer = new window.RTCSessionDescription({
            type: 'answer',
            sdp: sdp
        });
        this.peerConnection.setRemoteDescription(answer);
    }
};

class RemoteMedia {
    localSdp;
    remoteStream;
    videoDisplay;
    peerConnection;

    async open(videoId) {
        try {
            const configuration = { sdpSemantics: "plan-b" };

            this.videoDisplay = document.getElementById(videoId);
            this.peerConnection = new RTCPeerConnection(configuration);
            console.log('Created remote peer connection object remotePeerConnection.');
            this.peerConnection.addEventListener('iceconnectionstatechange', () => {
                switch (this.peerConnection.iceConnectionState) {
                    case 'checking':
                        console.log('@connectionstatechange, connecting');
                        break;
                    case 'connected':
                    case 'completed':
                        console.log('@connectionstatechange, connected');
                        break;
                    case 'failed':
                        console.log('@connectionstatechange, failed');
                        break;
                    case 'disconnected':
                        console.log('@connectionstatechange, disconnected');
                        break;
                    case 'closed':
                        console.log('@connectionstatechange, closed');
                        break;
                }
            }); 
            this.peerConnection.addEventListener('track', event => this.gotRemoteMediaStream(event));
            this.peerConnection.offer = await this.peerConnection.createOffer(offerOptions);
            console.log("createOffer", this.peerConnection.offer);
            await this.peerConnection.setLocalDescription(this.peerConnection.offer);
            this.localSdp = this.peerConnection.offer.sdp;
        } catch (e) {
            console.log(`getPublish error: ${e}`);
        }
    }

    setRemoteSdp(sdp) {
        console.log("set remote sdp: " + sdp);
        let answer = new window.RTCSessionDescription({
            type: 'answer',
            sdp: sdp
        });
        this.peerConnection.setRemoteDescription(answer);
    }

    gotRemoteMediaStream(event) {
        const mediaStream = event.streams[0];
        this.videoDisplay.srcObject = mediaStream;
        this.videoDisplay.style.visibility = 'visible';
        this.remoteStream = mediaStream;
        console.log('Remote peer connection received remote stream.');
    }
};

class RTCSession {
    media;
    streamId;
    ws;
    localSdp;
    remoteSdp;

    onPublishAck;
    onPlayAck;

    role;

    connected;

    constructor(role) {
        this.role = role
    }

    start(url, sdp) {
        this.localSdp = sdp;
        this.ws = new WebSocket(url);
        this.ws.onopen = (evt) => {
            this.onWsOpen(evt);
        };

        this.ws.onclose = (evt) => {
            this.onWsClose(evt);
        };

        this.ws.onmessage = (evt) => {
            this.onWsMessage(evt);
        }
    }

    send(method, data) {
        let request = {
            seq: 1,
            method: method,
            stream: this.streamId,
            data: data
        };

        let msg = JSON.stringify(request);
        this.ws.send(msg);

        console.log("send message: " + msg);
    }

    publish() {
        let data = {
            sdp: this.localSdp
        };
        this.send("stream.publish", data);
    }

    play() {
        let data = {
            sdp: this.localSdp
        };
        this.send("stream.play", data);
    }

    mute(muteAudio, muteVideo) {

    }

    close() {

    }

    onWsOpen(evt) {
        console.log("Connection open, role " + this.role);
        this.connected = true;
        if (this.role === "publisher") {
            this.publish();
        } else {
            this.play();
        }
        heartbeat();
    }

    onWsMessage(evt) {
        let data = evt.data;
        let jsonAck = JSON.parse(data);
        console.log("Receive message: " + data);
        
        if (jsonAck.err == 0) {
            console.log("sdp : " + jsonAck.data.sdp);
            this.media.setRemoteSdp(jsonAck.data.sdp);
        }
    }

    onWsClose(evt) {
        console.log("Connection closed ..."); 
        this.connected = false;
    }

    heartbeat() {
        if (!this.ws)
            return;
        if (this.ws.readyState !== 1)
            return;
        this.ws.send("heartbeat");
        setTimeout(() => this.heartbeat(), 500);
      }
}

let publisher = new RTCSession("publisher");
let player = new RTCSession("player");
let localMedia = new LocalMedia();
let remoteMedia = new RemoteMedia();


async function publish(url, streamId) {
    await localMedia.open("localVideo");
    publisher.streamId = streamId;

    publisher.media = localMedia;
    publisher.start(url, localMedia.localSdp);
}

async function play(url, streamId) {
    await remoteMedia.open("remoteVideo");
    player.streamId = streamId;

    player.media = remoteMedia;
    player.start(url, remoteMedia.localSdp);
}

let url = document.getElementById("url").value;

let btPublish = document.getElementById("publish");
let btPublishStop = document.getElementById("publishStop");

let btPlay = document.getElementById("play");
let btPlayStop = document.getElementById("playStop");

btPublish.onclick = function () {
    let publishStreamId = document.getElementById("publishStreamId").value;
    publish(url, publishStreamId);
};

btPublishStop.onclick = function () {
   // rtc.stopCamera();
};

btPlay.onclick = function () {
    let playStreamId = document.getElementById("playStreamId").value;
    play(url, playStreamId);
}

btPlayStop.onclick = function () {
   // rtc.stopCamera();
};
