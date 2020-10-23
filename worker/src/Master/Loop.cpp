#include "Master/Log.hpp"
#include "Master/Loop.hpp"

#define PMS_CLASS "pingos::Loop"

namespace pingos {

void Loop::ClassInit()
{
    DepLibUV::ClassInit();
}

uv_loop_t* Loop::FetchLoop()
{
    auto loop = DepLibUV::GetLoop();
    if (loop == nullptr || uWS::Loop::get(loop) == nullptr) {
        PMS_ERROR("Creating uWS loop failed.");
        return nullptr;
    }

    return loop;
}

void Loop::Run()
{
    uWS::Loop::get()->run();
}

}