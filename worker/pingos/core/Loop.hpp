#pragma once

#include "DepLibUV.hpp"
#include "App.h"

namespace pingos {

class Loop {
public:
    static void ClassInit();
    static uv_loop_t* FetchLoop();
    static void Run();

private:
    static uv_loop_t *loop;
};

}
