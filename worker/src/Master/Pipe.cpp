#include "Master/Pipe.hpp"

namespace pingos {

static int mpipe(int *fds)
{
    if (pipe(fds) == -1) {
        return -1;
    }

    if (fcntl(fds[0], F_SETFD, FD_CLOEXEC) == -1 ||
        fcntl(fds[1], F_SETFD, FD_CLOEXEC) == -1)
    {
        close(fds[0]);
        close(fds[1]);

        return -1;
    }

    return 0;
}

Pipe::Pipe(uv_loop_t *loop)
{
    int fdIn[2];
    int fdOut[2];

    ASSERT(mpipe(fdIn) != -1);
    ASSERT(mpipe(fdOut) != -1);

    ASSERT(uv_pipe_init(loop, &childIn, 0) == 0);
    ASSERT(uv_pipe_init(loop, &childOut, 0) == 0);
    ASSERT(uv_pipe_init(loop, &parentIn, 0) == 0);
    ASSERT(uv_pipe_init(loop, &parentOut, 0) == 0);

    ASSERT(uv_pipe_open(&childIn, fdIn[0]) == 0);
    ASSERT(uv_pipe_open(&parentIn, fdIn[1]) == 0);
    ASSERT(uv_pipe_open(&parentOut, fdOut[0]) == 0);
    ASSERT(uv_pipe_open(&childOut, fdOut[1]) == 0);

    ASSERT(uv_is_readable((uv_stream_t*) &childIn));
    ASSERT(uv_is_writable((uv_stream_t*) &childOut));
    ASSERT(uv_is_writable((uv_stream_t*) &parentIn));
    ASSERT(uv_is_readable((uv_stream_t*) &parentOut));

    int bidir = uv_is_writable((uv_stream_t*) &childIn);
    ASSERT(uv_is_readable((uv_stream_t*) &childOut) == bidir);
    ASSERT(uv_is_readable((uv_stream_t*) &parentIn) == bidir);
    ASSERT(uv_is_writable((uv_stream_t*) &parentOut) == bidir);
}

Pipe::~Pipe()
{

}

void Pipe::CloseCurrntProcessPipe()
{
   uv_close((uv_handle_t*) &childIn, nullptr);
   uv_close((uv_handle_t*) &childOut, nullptr);
}

}
