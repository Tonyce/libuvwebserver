#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//
#include <uv.h>

const int FIB_UNTIL = 10;
uv_loop_t* loop;

void fib(uv_work_t* req);
void after_fib(uv_work_t* req, int status);

int main(int argc, char* argv[]) {
    loop = uv_default_loop();

    int data[FIB_UNTIL];
    uv_work_t req[FIB_UNTIL];
    int i;
    for (i = 0; i < FIB_UNTIL; i++) {
        data[i] = i;
        req[i].data = (void*)&data[i];
        uv_queue_work(loop, &req[i], fib, after_fib);
    }

    return uv_run(loop, UV_RUN_DEFAULT);
}

void after_fib(uv_work_t* req, int status) {
    fprintf(stderr, "Done calculating %dth fibonacci\n", *(int*)req->data);
}

long fib_(long t) {
    if (t == 0 || t == 1)
        return 1;
    else
        return fib_(t - 1) + fib_(t - 2);
}

void fib(uv_work_t* req) {
    int n = *(int*)req->data;
    if (random() % 2) {
        sleep(1);
    } else {
        sleep(3);
    }
    long fib = fib_(n);
    fprintf(stderr, "%dth fibonacci is %lu\n", n, fib);
}