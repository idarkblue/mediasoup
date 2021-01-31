
#include "PMSMasterProcess.hpp"
#include "PMSSubprocess.hpp"

namespace pingos
{
    Subprocess* PMSMasterProcess::NewSubprocess(uv_loop_t *loop)
    {
        return new PMSSubprocess(loop);
    }

    void PMSMasterProcess::DeleteSubprocess(Subprocess *subprocess)
    {
        delete (PMSSubprocess*) subprocess;
    }

}
