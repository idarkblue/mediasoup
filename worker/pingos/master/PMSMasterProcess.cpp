
#include "PMSMasterProcess.hpp"
#include "PMSSubprocess.hpp"

namespace pingos
{
    SubprocessAgent* PMSMasterProcess::NewSubprocess(uv_loop_t *loop)
    {
        return new PMSSubprocess(loop);
    }

    void PMSMasterProcess::DeleteSubprocess(SubprocessAgent *subprocess)
    {
        delete (PMSSubprocess*) subprocess;
    }

}
