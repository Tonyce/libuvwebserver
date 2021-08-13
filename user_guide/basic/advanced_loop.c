#include <stdio.h>

//
#include <uv.h>

int64_t counter = 0;
uv_loop_t* loop;

void idle_cb(uv_idle_t* handle);
void prepare_cb(uv_prepare_t* handle);

int main(int argc, char* argv[]) {
    loop = uv_default_loop();

    uv_idle_t idler;
    uv_prepare_t prep;

    uv_idle_init(loop, &idler);
    uv_prepare_init(loop, &prep);

    uv_idle_start(&idler, idle_cb);
    uv_prepare_start(&prep, prepare_cb);

    uv_run(loop, UV_RUN_DEFAULT);

    return 0;
}

void prepare_cb(uv_prepare_t* handle) {
    printf("Prep cb\n");
}

void idle_cb(uv_idle_t* handle) {
    printf("Idle cb\n");
    counter += 1;

    if (counter >= 5) {
        uv_stop(loop);
        printf("uv_stop called\n");
    }
}