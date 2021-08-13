#include <stdio.h>

//
#include <uv.h>

uv_loop_t* loop;
uv_process_t child_req;

int main(int argc, char* argv[]) {
    loop = uv_default_loop();

    char* args[3] = {"sleep", "100", NULL};

    uv_process_options_t options = {0};
    options.exit_cb = NULL;
    options.file = "sleep";
    options.args = args;
    options.flags = UV_PROCESS_DETACHED;  // 让子进程脱离父进程

    int r;
    if ((r = uv_spawn(loop, &child_req, &options))) {
        fprintf(stderr, "%s\n", uv_strerror(r));
        return 1;
    }
    fprintf(stderr, "Launched sleep with PID %d\n", child_req.pid);
    // uv_unref((uv_handle_t*)&child_req);

    return uv_run(loop, UV_RUN_DEFAULT);
}