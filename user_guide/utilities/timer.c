#include <stdio.h>
#include <uv.h>

uv_loop_t* loop;

void timer_cb(uv_timer_t* handle);

int main(int argc, char* argv[]) {
    loop = uv_default_loop();

    uv_timer_t timer_req;
    uv_timer_init(loop, &timer_req);
    uv_timer_start(&timer_req, timer_cb, 5000, 2000);

    return uv_run(loop, UV_RUN_DEFAULT);
}

void timer_cb(uv_timer_t* handle) {
    printf("timer_cb\n");
};