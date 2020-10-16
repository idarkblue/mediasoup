using Newtonsoft.Json.Linq;
using UnityEngine;
using WebSocketSharp;

public class Request {
    Request(string method, string app, string uid, string stream) {
        m_version = "1.0";
        m_mehtod = method;
        m_app = app;
        m_uid = uid;
        m_stream = stream;
    }

    string m_version;
    string m_method;
    string m_app;
    string m_uid;
    string m_stream;
    JObject m_jsonData;

    void SetData(JObject &jsonData) {
        m_jsonData = jsonData;
    }

};

public class RtcSession {
    enum Role {
        PUBLISHER,
        PLAYER
    };

    WebSocket ws;

    RtcSession::Role m_role;
    string m_app;
    string m_uid;
    string m_stream;

    void Init(RtcSession::Role role, string app, string uid, string stream) {
        m_role = role;
        m_app = app;
        m_uid = uid;
        m_stream = stream;
    };

    int Connect(string url) {
        ws = new WebSocket(url);
        ws.OnMessage += (sender, e) =>
        {
            Debug.Log("收到响应的数据：" + e.Data);
        };

        ws.OnError += Ws_OnError;
        ws.OnClose += Ws_OnClose;
        ws.OnOpen += Ws_OnOpen;
        ws.OnRead += Ws_OnRead;
        ws.ConnectAsync();
    };

    /*
    发布流请求
    {
        "version": "1.0",
        "method": "stream.offer",
        "app": "xxx",
        "uid": "xxx",
        "stream": "xxx",
        "data": {
            "publish": true,
            "sdp": "sdp内容"
        }
    }
    */
    int Publish(string sdp) {
        if (m_role != Role::PUBLISHER) {
            // 角色为 player，不允许publish
            return -1;
        }

        JObject jsonObject = this->GeneratePublishRequest(sdp);
        ws->send(jsonObject);
        return 0;
    };

    /*
    播放请求
    {
        "version": "1.0",
        "method": "stream.offer",
        "app": "xxx",
        "uid": "xxx",
        "stream": "xxx",
        "data": {
            "publish": false,
            "sdp": "sdp内容"
        }
    }
    */
    int Play(string sdp) {
        if (m_role != Role::PLAYER) {
            // 角色为 publisher，不允许play
            return -1;
        }

        JObject jsonObject = this->GeneratePlayRequest(sdp);
        ws->send(jsonObject);
        return 0;
    };

    /* mute请求，audio 为true 表示屏蔽音频，video为true表示屏蔽视频
    {
        "version": "1.0",
        "method": "stream.mute",
        "app": "xxx",
        "uid": "xxx",
        "stream": "xxx",
        "data": {
            "video": false,
            "audio": false
        }
    }
    */
    int Mute(bool video, bool audio) {
        JObject jsonObject = this->GenerateMuteRequest(video, audio);
        ws->send(jsonObject);
        return 0;
    };

    /* 关闭请求
    {
        "version": "1.0",
        "method": "stream.close",
        "app": "xxx",
        "uid": "xxx",
        "stream": "xxxxx",
        "data": { }
    }
    */
    int Close() {
        JObject jsonObject = this->GenerateCloseRequest();
        ws->send(jsonObject);
        return 0;
    }

    void FillJsonCommon(string method, JObject &jsonObject) {
        jsonObject["version"] = "1.0";
        jsonObject["method"] = method;
        jsonObject["app"] = m_app;
        jsonObject["uid"] = m_uid;
        jsonObject["stream"] = m_stream;
    }

    JObject GenerateConnectRequest() {
        JObject jsonObject;
        FillJsonCommon("stream.connect", jsonObject);
        jsonObject["data"]["token"] = "测试用的token，目前版本可随意填写token";

        return jsonObject;
    }

    JObject GeneratePublishRequest(string sdp) {
        JObject jsonObject;
        FillJsonCommon("stream.offer", jsonObject);
        jsonObject["data"]["publish"] = true;
        jsonObject["data"]["sdp"] = sdp;

        return jsonObject;
    }

    JObject GeneratePlayRequest(string sdp) {
        JObject jsonObject;
        FillJsonCommon("stream.offer", jsonObject);
        jsonObject["data"]["publish"] = false;
        jsonObject["data"]["sdp"] = sdp;

        return jsonObject;
    }

    JObject GenerateMuteRequest(bool video, bool audio) {
        JObject jsonObject;
        FillJsonCommon("stream.mute", jsonObject);
        jsonObject["data"]["video"] = video; // true表示屏蔽video流
        jsonObject["data"]["audio"] = audio; // true表示屏蔽audio流

        return jsonObject;
    }

    private void Ws_OnOpen(object sender, System.EventArgs e)
    {
        Debug.Log("OnOpen");

        Request request = this->GenerateConnectRequest();

        JObject jsonObject;
        request.FillJson(jsonObject);

        ws.send(jsonObject);
    }

    private void Ws_OnClose(object sender, CloseEventArgs e)
    {
        Debug.Log("OnClose");
    }

    private void Ws_OnError(object sender, ErrorEventArgs e)
    {
        Debug.Log("OnError");
    }

    // 接收到的wss内容
    private void Ws_OnRead(string data)
    {
        JObject jsonObject(data);

        if (jsonObject["on_result"] == "stream.connected") {
            // stream.connect 返回内容
            Rtc_OnConnected(jsonObject);
        } else if (jsonObject["on_result"] == "stream.answer") {
            // stream.offer 返回内容
            Rtc_OnAnswer(jsonObject);
        } else if (jsonObject["on_result"] == "stream.muted") {
            // stream.mute 返回内容
            Rtc_OnMuted(jsonObject);
        } else if (jsonObject["on_result"] == "stream.closed") {
            // stream.close 返回内容
            Rtc_OnClosed(jsonObject);
        }
    }

    private void Rtc_OnConnected(JObject &jsonObject) {
        if (jsonObject["err"] != 0) {
            Error.log("执行失败，错误信息 errcode %d, errmsg %s ",
                jsonObject["err"], jsonObject["err_msg"]);
            return;
        }

        Info.log("连接成功！");
    }

    private void Rtc_OnAnswer(JObject &jsonObject) {
        // 向服务器发送 offer 的执行结果，根据 协议里的 err 字段判断是否成功
        if (jsonObject["err"] != 0) {
            Error.log("执行失败，错误信息 errcode %d, errmsg %s ",
                jsonObject["err"], jsonObject["err_msg"]);
            return;
        }

        string remoteSdp = jsonObject["data"]["sdp"];
        // TODO: set remote sdp
    }

    private void Rtc_OnMuted(JObject &jsonObject) {
        // mute执行结果，根据 协议里的 err 字段判断是否成功
        if (jsonObject["err"] != 0) {
            Error.log("执行失败，错误信息 errcode %d, errmsg %s ",
                jsonObject["err"], jsonObject["err_msg"]);
            return;
        }

        if (jsonObject["data"]["video"] == true) {
            Info.log("视频流处于开启状态");
        } else {
            Info.log("视频流已经被屏蔽")
        }

        if (jsonObject["data"]["audio"] == true) {
            Info.log("音频流处于开启状态");
        } else {
            Info.log("音频流已经被屏蔽")
        }
    }
    private void Rtc_OnClosed(JObject &jsonObject) {
        // webrtc连接断开，可以在这里回收为webrtc创建的资源
        if (jsonObject["err"] != 0) {
            Error.log("执行失败，错误信息 errcode %d, errmsg %s ",
                jsonObject["err"], jsonObject["err_msg"]);
            return;
        }

        Info.log("关闭成功");
    }

};

// 以推流为例

int main()
{
    // TODO:  sdp

    RtcSession session = new RtcSession();

    session.Init(RtcSession::Role::PUBLISHER, "app当前字段填写成固定字符串即可",
        "用户ID", "流ID，推流的流ID不可重复");
    session.Connect("wss://live.pingos.io:8000");

    session.Publish(sdp);

    session.Mute(false, true); // 将音频静音（服务器会屏蔽音频流）

    session.Close(); // 关闭
}