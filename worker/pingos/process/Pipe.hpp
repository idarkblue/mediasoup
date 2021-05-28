extern "C" {
    #include <uv.h>
    #include <unistd.h>
    #include <sys/wait.h>
    #include <errno.h>
    #include <fcntl.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>

}

namespace pingos {
#define ASSERT(expr)                                      \
 do {                                                     \
  if (!(expr)) {                                          \
    fprintf(stderr,                                       \
            "Assertion failed in %s on line %d: %s\n",    \
            __FILE__,                                     \
            __LINE__,                                     \
            #expr);                                       \
    abort();                                              \
  }                                                       \
 } while (0)

class Pipe {
public:
    Pipe(uv_loop_t *loop);
    virtual ~Pipe();

    void CloseCurrntProcessPipe();

    uv_pipe_t childIn;
    uv_pipe_t childOut;
    uv_pipe_t parentIn;
    uv_pipe_t parentOut;
};
}