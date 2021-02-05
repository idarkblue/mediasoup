#include "PMSSubprocess.hpp"

namespace pingos
{
    PMSSubprocess::PMSSubprocess(uv_loop_t *loop) : SubprocessAgent(loop)
    {

    }

    void PMSSubprocess::ReceiveChannelMessage(std::string_view &payload)
    {

    }

    int PMSSubprocess::ReceiveMasterMessage(std::string &payload)
    {
        return 0;
    }

}
