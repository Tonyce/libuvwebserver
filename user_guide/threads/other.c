#include <stdio.h>

//
#include <uv.h>

static uv_once_t once_only = UV_ONCE_INIT;

int i = 0;

void increment() {
    i++;
}

void thread1() {
    /* ... work */
    uv_once(&once_only, increment);
}

void thread2() {
    /* ... work */
    uv_once(&once_only, increment);
}

int main(int argc, char* argv[]) {
    int tracklen = 10;
    uv_thread_t hare_id;
    uv_thread_t tortoise_id;

    uv_thread_create(&hare_id, thread1, &tracklen);
    uv_thread_create(&tortoise_id, thread2, &tracklen);

    uv_thread_join(&hare_id);
    uv_thread_join(&tortoise_id);

    printf("i: %d\n", i);

    return 0;
}