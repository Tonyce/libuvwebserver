#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//
#include <uv.h>

const int FIB_UNTIL = 10;

uv_loop_t* loop;
uv_work_t fib_reqs[FIB_UNTIL];

void fib(uv_work_t* req);
void after_fib(uv_work_t* req, int status);
void signal_handler(uv_signal_t* req, int signum);

int main(int argc, char* argv[]) {
    loop = uv_default_loop();

    int data[FIB_UNTIL];

    int i;
    for (i = 0; i < FIB_UNTIL; i++) {
        data[i] = i;
        fib_reqs[i].data = (void*)&data[i];
        uv_queue_work(loop, &fib_reqs[i], fib, after_fib);
    }

    uv_signal_t sig;
    uv_signal_init(loop, &sig);
    uv_signal_start(&sig, signal_handler, SIGINT);

    return uv_run(loop, UV_RUN_DEFAULT);
}

void signal_handler(uv_signal_t* req, int signum) {
    printf("Signal received\n");
    int i;
    for (i = 0; i < FIB_UNTIL; i++) {
        uv_cancel((uv_req_t*)&fib_reqs[i]);
    }
    uv_signal_stop(req);
};

void after_fib(uv_work_t* req, int status) {
    if (status == UV_ECANCELED)
        fprintf(stderr, "Canceled calculating %dth fibonacci\n", *(int*)req->data);
    else
        fprintf(stderr, "Done calculating %dth fibonacci with status %d\n", *(int*)req->data, status);
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