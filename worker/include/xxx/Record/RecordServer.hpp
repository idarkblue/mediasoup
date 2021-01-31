#pragma once

#include <map>
#include <uv.h>
#include "NetServer.hpp"
#include "json.hpp"
#include "Record/RecordRequest.hpp"

using json = nlohmann::json;

namespace pingos {

class RecordServer : public NetServer::Listener {
public:
    struct Context {
        uv_process_options_t options;
        uv_process_t         process;
        bool recordDone { false };
        std::string streamId;
        std::string filePath;
        void *data;
    };
public:
    RecordServer() = default;
    RecordServer(uv_loop_t *loop);
    virtual ~RecordServer();

    int StartRecord(RecordRequest &request);
    int StopRecord(RecordRequest &request);

// Implement NetServer::Listener
protected:
    virtual int OnMessage(NetConnection *connection) override;
    virtual void OnDisconnect(NetConnection *connection) override;

protected:
    int Exec(std::string file, char **args, RecordServer::Context *ctx);
    void OnProcessExited(uv_process_t *req, int64_t status, int termSignal);
    int ExecRecordDone(RecordServer::Context *ctx);

private:
    std::map<std::string, Context*> jobMap;
    uv_loop_t *uvLoop;
};

}
