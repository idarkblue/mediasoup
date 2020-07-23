#include "Master/Loop.hpp"

namespace Master {

void Loop::ClassInit()
{
    DepLibUV::ClassInit();
}

uv_loop_t* Loop::FetchLoop()
{
    auto loop = DepLibUV::GetLoop();
    if (loop == nullptr || uWS::Loop::get(loop) == nullptr) {
        fprintf(stderr, "Creating uWS loop failed.\r\n");
        return nullptr;
    }

    return loop;
}

void Loop::Run()
{
    uWS::Loop::get()->run();
}

}