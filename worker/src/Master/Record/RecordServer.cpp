#define PMS_CLASS "pingos::RecordServer"
#define MS_CLASS "pingos::RecordServer"


#include <algorithm>
#include <functional>
#include "Master/Defines.hpp"
#include "Master/Log.hpp"
#include "MediaSoupErrors.hpp"
#include "Master/Configuration.hpp"
#include "Record/RecordServer.hpp"
#include "Rtsp/RtspHeader.hpp"

namespace pingos {

RecordServer::RecordServer(uv_loop_t *loop):
    uvLoop(loop)
{
}

RecordServer::~RecordServer()
{

}

/*
{
    method: record.start
    stream: xxx
    data: {
        targetHost: xxx,
        targetPort: xxx
    }
}
*/
int RecordServer::OnMessage(NetConnection *nc)
{
    std::string message;
    nc->PopData(message);

    json jsonRoot = json::parse(message);
    std::string method;
    std::string stream;

    RecordRequest request;
    try {
        request = RecordRequest(nc, jsonRoot);

        switch (request.methodId) {
            case RecordRequest::MethodId::START_RECORD:
            this->StartRecord(request);
            break;

            case RecordRequest::MethodId::STOP_RECORD:
            this->StopRecord(request);
            break;

            default:
            break;
        }

    } catch (MediaSoupError &error) {
        PMS_ERROR("Record error, {}", error.what());
        request.Error(error.what());
    }
    

    return 0;
}

void RecordServer::OnDisconnect(NetConnection *nc)
{
    return;
}

int RecordServer::StartRecord(RecordRequest &request)
{
    std::string host, fileName;
    uint16_t port;

    JSON_READ_VALUE_DEFAULT(request.jsonData, "targetHost", std::string, host, Configuration::record.targetHost);
    JSON_READ_VALUE_DEFAULT(request.jsonData, "targetPort", uint16_t, port, Configuration::record.targetPort);
    JSON_READ_VALUE_DEFAULT(request.jsonData, "fileName", std::string, fileName, request.stream + ".webm");

    std::string url = std::string("rtsp://") + host + std::string(":") + std::to_string(port) + std::string("/") + request.stream;
    std::string file = Configuration::record.recordPath + fileName;

    Context *ctx = new Context();
    ctx->recordDone = false;
    ctx->streamId = request.stream;
    ctx->filePath = file;

    std::string cmd = std::string("ffmpeg -y -i ");
    cmd += url + std::string(" ");
    cmd += std::string("-c copy ");
    cmd += file;

    std::vector<std::string> strVec;
    RtspHeaderLines::SplitString(cmd, ' ', strVec);

    char **args = new char*[strVec.size() + 1];

    size_t i = 0;
    for (i = 0; i < strVec.size(); i++) {
        args[i] = (char*) strVec[i].c_str();
    }
    args[i] = nullptr;

    if (this->Exec("ffmpeg", args, ctx) != 0) {
        PMS_ERROR("StreamId[{}] exec failed.", ctx->streamId);
        delete[] args;
        return -1;
    }

    this->jobMap[ctx->streamId] = ctx;

    delete[] args;

    PMS_INFO("StreamId[{}] exec success.", ctx->streamId);

    request.Accept();

    return 0;
}

int RecordServer::StopRecord(RecordRequest &request)
{
    std::string streamId = request.stream;
    if (this->jobMap.find(streamId) == this->jobMap.end() || !this->jobMap[streamId]) {
        PMS_ERROR("StreamId[{}] job not found.", streamId);
        request.Error("job not found");
        return -1;
    }

    auto ctx = this->jobMap[streamId];
    uv_process_kill(&ctx->process, SIGINT);
    this->jobMap.erase(streamId);

    PMS_INFO("StreamId[{}] close job process.", streamId);
    request.Accept();

    return 0;
}

int RecordServer::Exec(std::string file, char **args, RecordServer::Context *ctx)
{
    char *env[1] = { nullptr };

    std::string cmdString = "";
    for(int i = 0; args[i]; i++) {
        cmdString += std::string(args[i]) + std::string(" ");
    }

    PMS_INFO("exec: {}", cmdString);

    uv_stdio_container_t childStdio[3];
    childStdio[0].flags = UV_IGNORE;
    childStdio[0].data.fd = 0;

    childStdio[1].flags = UV_INHERIT_FD;
    childStdio[1].data.fd = 1;

    childStdio[2].flags = UV_INHERIT_FD;
    childStdio[2].data.fd = 2;

    ctx->options.stdio = childStdio;
    ctx->options.stdio_count = sizeof(childStdio) / sizeof(uv_stdio_container_t);

    ctx->data = this;
    ctx->options.exit_cb = [](uv_process_t *req, int64_t status, int termSignal) {
        auto ctx = static_cast<RecordServer::Context*>(req->data);
        RecordServer *server = (RecordServer*) ctx->data;
        server->OnProcessExited(req, status, termSignal);
    };
    ctx->options.file = file.c_str();

    ctx->options.args = args;

    ctx->options.env = env;
    ctx->options.flags = 0;
    ctx->options.cwd = nullptr;

    ctx->process.data = ctx;

    int ret = 0;
    int maxRetries = 5;
    int count = 0;
    do {
        ret = ::uv_spawn(this->uvLoop, &ctx->process, &ctx->options);
        if (ret != 0) {
            PMS_ERROR("Spawn [file {}] {} {}", file, ret, ::uv_err_name(ret));
            //return -1;
        } else {
            PMS_INFO("Spawn [file {}] success\n", file);
        }
    } while (ret != 0 && ++count <= maxRetries);

    if (ret != 0) {
        return -1;
    }

    return 0;
}

void RecordServer::OnProcessExited(uv_process_t *req, int64_t status, int termSignal)
{
    Context *ctx = (Context*) req->data;
    if (!ctx) {
        return;
    }

    bool recordDone = ctx->recordDone;

    std::string streamId = ctx->streamId;
    std::string file = ctx->filePath;

    delete ctx;

    if (!recordDone) {
        ctx = new Context();
        ctx->recordDone = true;
        ctx->streamId = streamId;
        ctx->filePath = file;

        if (this->ExecRecordDone(ctx) != 0) {
            PMS_ERROR("StreamId[{}] exec record done failed.", ctx->streamId);
        }

        return;
    }

    return;
}

int RecordServer::ExecRecordDone(RecordServer::Context *ctx)
{
    if (Configuration::record.execRecordDone.empty()) {
        return 0;
    }

    std::vector<std::string> strVec;
    RtspHeaderLines::SplitString(Configuration::record.execRecordDone, ' ', strVec);
    char **args = new char*[strVec.size() + 1];
    int i = 0;
    for (auto &str : strVec) {
        if (str == "$file") {
            args[i] = (char*) ctx->filePath.c_str();
        } else {
            args[i] = (char*) str.c_str();
        }
        i++;
    }

    args[i] = nullptr;

    if (this->Exec(strVec[0], args, ctx) != 0) {
        PMS_ERROR("StreamId[{}] exec failed", ctx->streamId);
        return -1;
    }

    delete[] args;

    return 0;
}

}
