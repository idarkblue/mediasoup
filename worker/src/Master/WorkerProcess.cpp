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

    m_pipeFile = WorkerProcess::pipePath + "." + std::to_string(opt.slot);
}

WorkerProcess::~WorkerProcess()
{
    delete m_pipeClient;
    delete m_pipeServer;
}

int WorkerProcess::SetPipe()
{
    m_pipeServer = new PipeServer(m_opt.loop);
    m_pipeServer->SetListener(this);
    m_pipeServer->Listen(m_pipeFile);

    m_pipeClient = new PipeClient(m_opt.loop);
    m_pipeClient->Connect(m_pipeFile);

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

    uv_stdio_container_t childStdio[5];
    childStdio[0].flags = UV_IGNORE;
    childStdio[0].data.fd = 0;

    childStdio[1].flags = UV_INHERIT_FD;
    childStdio[1].data.fd = 1;

    childStdio[2].flags = UV_INHERIT_FD;
    childStdio[2].data.fd = 2;

    childStdio[3].flags = static_cast<uv_stdio_flags>(UV_INHERIT_STREAM);
    childStdio[3].data.stream = (uv_stream_t *)(m_pipeClient->GetPipeHandle());

    childStdio[4].flags = static_cast<uv_stdio_flags>(UV_INHERIT_STREAM);
    childStdio[4].data.stream = (uv_stream_t *)(m_pipeClient->GetPipeHandle());

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

void WorkerProcess::ReceiveMessage(std::string &payload)
{
    m_channel->SendBinary((const uint8_t *) payload.c_str(), payload.length());
}

void WorkerProcess::OnChannelAccept(UnixStreamSocket *channel)
{
    PMS_INFO("accept ...................");
    m_channel = channel;
}

void WorkerProcess::OnChannelClosed(UnixStreamSocket *channel)
{

}

void WorkerProcess::OnChannelRecv(UnixStreamSocket *channel, std::string_view &payload)
{
}

}
