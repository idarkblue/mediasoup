#include "MediaSoupErrors.hpp"
#include "Loop.hpp"

#define MS_CLASS "pingos::Loop"

namespace pingos {

uv_loop_t *Loop::loop = nullptr;

void Loop::ClassInit()
{
    DepLibUV::ClassInit();
    FetchLoop();
}

uv_loop_t* Loop::FetchLoop()
{
    loop = DepLibUV::GetLoop();
    if (loop == nullptr || uWS::Loop::get(loop) == nullptr) {
        MS_THROW_ERROR("Creating uWS loop failed.");
        return nullptr;
    }

    return loop;
}

void Loop::Run()
{
    uWS::Loop::get()->run();
}

}