#include "RtcMaster.hpp"
#include "RtcWorker.hpp"

namespace pingos
{
    RtcMaster *RtcMaster::instance = nullptr;

    SubProcess* RtcMaster::NewProcess(uv_loop_t *loop)
    {
        return new RtcWorker(loop);
    }

    void RtcMaster::DeleteProcess(SubProcess *subprocess)
    {
        delete (RtcWorker *) subprocess;
    }

    void RtcMaster::OnProcessExited(SubProcess *subprocess)
    {

    }

    RtcMaster* RtcMaster::Instance()
    {
        if (!instance) {
            instance = new RtcMaster();
        }

        return instance;
    }
}
