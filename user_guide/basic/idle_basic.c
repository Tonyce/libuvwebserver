#include <stdio.h>
#include <uv.h>

int64_t counter = 0;

void wait_for_a_while(uv_idle_t* handle) {
    counter++;
    if (counter >= 10e3) {
        uv_idle_stop(handle);
    }
}

int main(int argc, char* argv[]) {
    uv_idle_t idler;

    uv_idle_init(uv_default_loop(), &idler);
    uv_idle_start(&idler, wait_for_a_while);

    printf("Ideling...\n");
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    printf("run over..\n");

    uv_loop_close(uv_default_loop());
    return 0;
}