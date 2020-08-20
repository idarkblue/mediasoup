#define PMS_CLASS "Master::WorkerProcess"
#include "Master/Log.hpp"
#include "Master/WorkerProcess.hpp"

namespace Master {

static char MEDIASOUP_VERSION_STRING[] = "MEDIASOUP_VERSION=3.1.15";
char **WorkerProcess::processArgs = nullptr;
std::string WorkerProcess::pipePath = "pms.sock";

void WorkerProcess::ClassInit(const char **args, const char *pipePath)
{
    int    count = 0;
    size_t length = 0;

    for (count = 0; args && args[count]; count++) {
        length = length > strlen(args[count]) ? length : strlen(args[count]);
    }

    processArgs = new char *[count + 1];
    processArgs[count] = nullptr;

    for (int i = 0; args && args[i]; i++) {
        processArgs[i] = new char[length + 1];
        snprintf(processArgs[i], length + 1, "%s", args[i]);
    }

    WorkerProcess::pipePath = pipePath;
}

WorkerProcess::WorkerProcess(Options &opt)
{
    m_opt = opt;

    m_pipeFile = WorkerProcess::pipePath + ".channel." + std::to_string(opt.slot);
    m_pipePayloadFile = WorkerProcess::pipePath + ".payload." + std::to_string(opt.slot);
}

WorkerProcess::~WorkerProcess()
{
    delete m_pipeClient[0];
    delete m_pipeClient[1];
    delete m_pipeClient[2];
    delete m_pipeClient[3];

    delete m_pipeServer[0];
    delete m_pipeServer[1];
}

int WorkerProcess::SetPipe()
{
// channel
    m_pipeServer[0] = new PipeServer(m_opt.loop);
    m_pipeServer[0]->SetListener(this);
    m_pipeServer[0]->Listen(m_pipeFile);

    m_pipeClient[0] = new PipeClient(m_opt.loop);
    m_pipeClient[0]->Connect(m_pipeFile);

    m_pipeClient[1] = new PipeClient(m_opt.loop);
    m_pipeClient[1]->Connect(m_pipeFile);

// payload channel
    m_pipeServer[1] = new PipeServer(m_opt.loop);
    m_pipeServer[1]->SetListener(this);
    m_pipeServer[1]->Listen(m_pipePayloadFile);

    m_pipeClient[2] = new PipeClient(m_opt.loop);
    m_pipeClient[2]->Connect(m_pipePayloadFile);

    m_pipeClient[3] = new PipeClient(m_opt.loop);
    m_pipeClient[3]->Connect(m_pipePayloadFile);

    return 0;
}

int WorkerProcess::Run()
{
    this->SetPipe();
    this->Spawn();

    return 0;
}

int WorkerProcess::Spawn()
{
    static char *env[] = {
        MEDIASOUP_VERSION_STRING,
        nullptr
    };

    uv_stdio_container_t childStdio[7];
    childStdio[0].flags = UV_IGNORE;
    childStdio[0].data.fd = 0;

    childStdio[1].flags = UV_INHERIT_FD;
    childStdio[1].data.fd = 1;

    childStdio[2].flags = UV_INHERIT_FD;
    childStdio[2].data.fd = 2;

    childStdio[3].flags = static_cast<uv_stdio_flags>(UV_INHERIT_STREAM);
    childStdio[3].data.stream = (uv_stream_t *)(m_pipeClient[0]->GetPipeHandle());

    childStdio[4].flags = static_cast<uv_stdio_flags>(UV_INHERIT_STREAM);
    childStdio[4].data.stream = (uv_stream_t *)(m_pipeClient[1]->GetPipeHandle());

    childStdio[5].flags = static_cast<uv_stdio_flags>(UV_INHERIT_STREAM);
    childStdio[5].data.stream = (uv_stream_t *)(m_pipeClient[2]->GetPipeHandle());

    childStdio[6].flags = static_cast<uv_stdio_flags>(UV_INHERIT_STREAM);
    childStdio[6].data.stream = (uv_stream_t *)(m_pipeClient[3]->GetPipeHandle());

    m_options.stdio = childStdio;
    m_options.stdio_count = sizeof(childStdio) / sizeof(uv_stdio_container_t);

    m_options.exit_cb = [](uv_process_t *req, int64_t status, int termSignal) {
        auto me = static_cast<Master::WorkerProcess*>(req->data);
        me->OnWorkerExited(req, status, termSignal);
    };
    m_options.file = m_opt.file.c_str();
    m_options.args = processArgs;
    m_options.env = env;
    m_options.flags = 0;

    PMS_INFO("Startting worker[{}] .....\n", m_opt.file);

    m_process.data = this;
    ::uv_spawn(m_opt.loop, &m_process, &m_options);

    return 0;
}

void WorkerProcess::OnWorkerExited(uv_process_t *req, int64_t status, int termSignal)
{
    PMS_ERROR("Worker process[{}] exited\r\n", req->pid);
    if (m_opt.listener) {
        m_opt.listener->OnWorkerProcessExited(this);
    }
}

void WorkerProcess::ProcessMessage(std::string_view &message)
{

}

void WorkerProcess::OnChannelAccept(PipeServer *ps, UnixStreamSocket *channel)
{
//    PMS_INFO("accept ...");

    if (ps == m_pipeServer[0]) {
        if (channel->GetRole() == ::UnixStreamSocket::Role::PRODUCER) {
            m_channel[0] = channel;
        } else if (channel->GetRole() == ::UnixStreamSocket::Role::CONSUMER) {
            m_channel[1] = channel;
        }
    } else if (ps == m_pipeServer[1]) {
        if (channel->GetRole() == ::UnixStreamSocket::Role::PRODUCER) {
            m_channel[2] = channel;
        } else if (channel->GetRole() == ::UnixStreamSocket::Role::CONSUMER) {
            m_channel[3] = channel;
        }
    } else {
        return;
    }
}

void WorkerProcess::OnChannelClosed(PipeServer *ps, UnixStreamSocket *channel)
{
    
}

void WorkerProcess::OnChannelRecv(PipeServer *ps, UnixStreamSocket *channel, std::string_view &payload)
{
    switch (payload[0]) {
        // 123 = '{' (a Channel JSON messsage).
        case 123:
            PWS_DEBUG("Channel message: {}", payload);
            this->ProcessMessage(payload);
            break;

        // 68 = 'D' (a debug log).
        case 68:
            payload.remove_prefix(1);
            PWS_DEBUG("[worker {}] => {}", m_process.pid, payload);
            break;

        // 87 = 'W' (a warn log).
        case 87:
            payload.remove_prefix(1);
            PWS_WARN("[worker {}] => {}", m_process.pid, payload);
            break;

        // 69 = 'E' (an error log).
        case 69:
            payload.remove_prefix(1);
            PWS_ERROR("[worker {}] => {}", m_process.pid, payload);
            break;

        // 88 = 'X' (a dump log).
        case 88:
            payload.remove_prefix(1);
            PWS_INFO("[worker {}] => {}", m_process.pid, payload);
            // eslint-disable-next-line no-console
            break;

        default:
            payload.remove_prefix(1);
            PWS_TRACE("[worker {}] => {}", m_process.pid, payload);
            // eslint-disable-next-line no-console
    }
}

int WorkerProcess::CreateRouter(const char *routerId)
{
    CreateRouterRequest request(routerId);

    this->Send2Process(&request);

    return 0;
}

int WorkerProcess::CreateWebRtcTransport()
{
    return 0;
}

int WorkerProcess::CreatePlainTransport()
{
    return 0;
}

int WorkerProcess::CreateProducer()
{
    return 0;
}

int WorkerProcess::CreateConsumer()
{
    return 0;
}

int WorkerProcess::Send2Process(ProcessRequest *request)
{
    std::string message = request->ToString();

    m_channel[0]->SendString(message);

    return 0;
}

}
