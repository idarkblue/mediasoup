#define PMS_CLASS "pingos::Worker"
#include "Master/Log.hpp"
#include "Master/Configuration.hpp"
#include "Master/Worker.hpp"

namespace pingos {

static char MEDIASOUP_VERSION_STRING[] = "MEDIASOUP_VERSION=3.6.24";

Worker::Worker(uv_loop_t *loop) :
    loop(loop), channelPipe(loop), payloadChannelPipe(loop)
{
}

Worker::~Worker()
{
}

int Worker::InitChannels()
{
    this->channelIn = new pingos::UnixStreamSocket(&this->channelPipe.parentIn, this, ::UnixStreamSocket::Role::CONSUMER);
    this->channelOut = new pingos::UnixStreamSocket(&this->channelPipe.parentOut, this, ::UnixStreamSocket::Role::PRODUCER);

    this->payloadChannelIn = new pingos::UnixStreamSocket(&this->payloadChannelPipe.parentIn, this, ::UnixStreamSocket::Role::CONSUMER);
    this->payloadChannelOut = new pingos::UnixStreamSocket(&this->payloadChannelPipe.parentOut, this, ::UnixStreamSocket::Role::PRODUCER);

    return 0;
}

int Worker::Start(int slot, std::string file)
{
    this->slot = slot;
    this->file = file;

    std::vector<std::string> vecArgs;

    vecArgs.push_back(file);

    for (auto &tag : Configuration::log.workerTags) {
        vecArgs.push_back(std::string("--logTags=") + tag);
    }

    if (!Configuration::log.workerLevel.empty()) {
        vecArgs.push_back(std::string("--logLevel=") + Configuration::log.workerLevel);
    }

    if (Configuration::webrtc.maxPort) {
        vecArgs.push_back(std::string("--rtcMaxPort=") + std::to_string(Configuration::webrtc.maxPort));
    }

    if (Configuration::webrtc.minPort) {
        vecArgs.push_back(std::string("--rtcMinPort=") + std::to_string(Configuration::webrtc.minPort));
    }

    if (!Configuration::webrtc.dtlsCertificateFile.empty() &&
        !Configuration::webrtc.dtlsPrivateKeyFile.empty())
    {
        vecArgs.push_back(std::string("--dtlsCertificateFile=") + Configuration::webrtc.dtlsCertificateFile);
        vecArgs.push_back(std::string("--dtlsPrivateKeyFile=") + Configuration::webrtc.dtlsPrivateKeyFile);
    }

    size_t length = 0;

    for (auto &item : vecArgs) {
        length = length > item.length() ? length : item.length();
        length += 1;
    }

    this->args = new char *[vecArgs.size() + 1];

    size_t i = 0;

    for (auto &arg : vecArgs) {
        this->args[i] = new char[length];
        snprintf(this->args[i], length + 1, "%s", arg.c_str());
        i++;
    }

    this->args[vecArgs.size()] = nullptr;

    this->InitChannels();

    return this->Spawn();
}

int Worker::Spawn()
{
    env[0] = MEDIASOUP_VERSION_STRING;
    env[1] = nullptr;

    childStdio[0].flags = UV_IGNORE;
    childStdio[0].data.fd = 0;

    childStdio[1].flags = UV_INHERIT_FD;
    childStdio[1].data.fd = 1;

    childStdio[2].flags = UV_INHERIT_FD;
    childStdio[2].data.fd = 2;

    childStdio[3].flags = static_cast<uv_stdio_flags>(UV_INHERIT_STREAM);
    childStdio[3].data.stream = (uv_stream_t *)(&this->channelPipe.childIn);

    childStdio[4].flags = static_cast<uv_stdio_flags>(UV_INHERIT_STREAM);
    childStdio[4].data.stream = (uv_stream_t *)(&this->channelPipe.childOut);

    childStdio[5].flags = static_cast<uv_stdio_flags>(UV_INHERIT_STREAM);
    childStdio[5].data.stream = (uv_stream_t *)(&this->payloadChannelPipe.childIn);

    childStdio[6].flags = static_cast<uv_stdio_flags>(UV_INHERIT_STREAM);
    childStdio[6].data.stream = (uv_stream_t *)(&this->payloadChannelPipe.childOut);

    this->options.stdio = childStdio;
    this->options.stdio_count = 7;// sizeof(childStdio) / sizeof(uv_stdio_container_t);

    this->options.exit_cb = [](uv_process_t *req, int64_t status, int termSignal) {
        auto me = static_cast<Worker*>(req->data);
        me->OnWorkerExited(req, status, termSignal);
    };
    this->options.file = this->file.c_str();

    this->options.args = this->args;

    this->options.env = env;
    this->options.flags = 0;

    this->process.data = this;

    int ret = 0;
    int maxRetries = 5;
    int count = 0;
    do {
        ret = ::uv_spawn(this->loop, &this->process, &this->options);
        if (ret != 0) {
            PMS_ERROR("Spawn [worker-{}][file {}] {} {}",
                this->slot, this->file, ret, ::uv_err_name(ret));
            //return -1;
        } else {
            PMS_INFO("Spawn [worker-{}][file {}] success\n",
                this->slot, this->file);
        }
    } while (ret != 0 && ++count <= maxRetries);

    if (ret != 0) {
        return -1;
    }

//    this->channelPipe.CloseCurrntProcessPipe();
//    this->payloadChannelPipe.CloseCurrntProcessPipe();

    return 0;
}

void Worker::OnWorkerExited(uv_process_t *req, int64_t status, int termSignal)
{
    PMS_ERROR("Worker process[{}] exited\r\n", req->pid);
    if (this->listener) {
        this->listener->OnWorkerExited(this);
    }
}

void Worker::OnChannelMessage(pingos::UnixStreamSocket* channel, std::string_view &payload)
{
    switch (payload[0]) {
        // 123 = '{' (a Channel JSON messsage).
        case 123:
            PMS_DEBUG("[worker-{} {}] Channel message: {}", this->slot, this->process.pid, payload);
            this->ReceiveChannelMessage(payload);
            break;

        // 68 = 'D' (a debug log).
        case 68:
            payload.remove_prefix(1);
            PMS_DEBUG("[worker-{} {}] => {}", this->slot, this->process.pid, payload);
            break;

        // 87 = 'W' (a warn log).
        case 87:
            payload.remove_prefix(1);
            PMS_WARN("[worker-{} {}] => {}", this->slot, this->process.pid, payload);
            break;

        // 69 = 'E' (an error log).
        case 69:
            payload.remove_prefix(1);
            PMS_ERROR("[worker-{} {}] => {}", this->slot, this->process.pid, payload);
            break;

        // 88 = 'X' (a dump log).
        case 88:
            payload.remove_prefix(1);
            PMS_INFO("[worker-{} {}] => {}", this->slot, this->process.pid, payload);
            // eslint-disable-next-line no-console
            break;

        default:
            payload.remove_prefix(1);
            PMS_TRACE("[worker-{} {}] => {}", this->slot, this->process.pid, payload);
            // eslint-disable-next-line no-console
    }
}

void Worker::OnChannelClosed(pingos::UnixStreamSocket* channel)
{

}

int Worker::ChannelSend(std::string data)
{
    if (this->channelOut) {
        this->channelOut->SendString(data);
        PMS_DEBUG("[Worker-{} {}] Send Data: {}", this->slot, this->process.pid, data);
    } else {
        PMS_ERROR("[Worker-{} {}] channel ptr is null", this->slot, this->process.pid);
    }

    return 0;
}

}
