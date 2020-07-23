#ifndef _MASTER_LOOP_HPP_INCLUDE_
#define _MASTER_LOOP_HPP_INCLUDE_

#include "DepLibUV.hpp"
#include "App.h"

namespace Master {

class Loop {
public:
    static void ClassInit();
    static uv_loop_t* FetchLoop();
    static void Run();
};

}

#endif
