#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

uv_loop_t* loop;

struct child_worker_s {
    uv_process_t req;
    uv_process_options_t options;
    uv_pipe_t pipe;
};

typedef struct child_worker_s child_worker_t;

child_worker_t* workers;
int round_robin_counter;
int child_worker_count;

uv_buf_t dummy_buf;
char worker_path[500];

void setup_workers();
void on_new_connection(uv_stream_t* server, int status);
void close_process_handle(uv_process_t* req, int64_t exit_status, int term_signal);

int main(int argc, char* argv[]) {
    dummy_buf = uv_buf_init("a", 1);
    loop = uv_default_loop();
    setup_workers();

    uv_tcp_t server;
    uv_tcp_init(loop, &server);

    struct sockaddr_in bind_addr;
    uv_ip4_addr("0.0.0.0", 7000, &bind_addr);
    uv_tcp_bind(&server, (const struct sockaddr*)&bind_addr, 0);

    int r;
    if ((r = uv_listen((uv_stream_t*)&server, 128, on_new_connection))) {
        fprintf(stderr, "Listen error %s\n", uv_err_name(r));
        return 2;
    }

    return uv_run(loop, UV_RUN_DEFAULT);
}

void setup_workers() {
    size_t path_size = 500;
    uv_exepath(worker_path, &path_size);
    strcpy(worker_path + (strlen(worker_path) - strlen("multi_echo_server")), "multi_echo_server_worker");
    fprintf(stderr, "Worker path: %s\n", worker_path);

    char* args[2] = {worker_path, NULL};

    round_robin_counter = 0;

    uv_cpu_info_t* info;
    int cpu_count;
    uv_cpu_info(&info, &cpu_count);
    uv_free_cpu_info(info, cpu_count);

    child_worker_count = cpu_count;
    workers = calloc(sizeof(child_worker_t), cpu_count);

    while (cpu_count--) {
        child_worker_t* worker = &workers[cpu_count];
        uv_pipe_init(loop, &worker->pipe, 1 /* ipc */);

        uv_stdio_container_t child_stdio[3];
        child_stdio[0].flags = UV_CREATE_PIPE | UV_READABLE_PIPE;
        child_stdio[0].data.stream = (uv_stream_t*)&worker->pipe;
        child_stdio[1].flags = UV_IGNORE;
        child_stdio[2].flags = UV_INHERIT_FD;
        child_stdio[2].data.fd = 2;

        worker->options.stdio = child_stdio;
        worker->options.stdio_count = 3;

        worker->options.exit_cb = close_process_handle;
        worker->options.file = args[0];
        worker->options.args = args;

        uv_spawn(loop, &worker->req, &worker->options);
        fprintf(stderr, "Started worker %d\n", worker->req.pid);
    }
};

void on_new_connection(uv_stream_t* server, int status) {
    if (status == -1) {
        return;
    }

    uv_tcp_t* client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, client);

    if (uv_accept(server, (uv_stream_t*)client) == 0) {
        uv_write_t* write_req = malloc(sizeof(uv_write_t));
        child_worker_t* worker = &workers[round_robin_counter];
        uv_write2(write_req, (uv_stream_t*)&worker->pipe, &dummy_buf, 1, (uv_stream_t*)client, NULL);
        round_robin_counter = (round_robin_counter + 1) % child_worker_count;
    }
    uv_close((uv_handle_t*)client, NULL);
};

void close_process_handle(uv_process_t* req, int64_t exit_status, int term_signal) {
    fprintf(stderr, "Process exited with status %" PRId64 ", signal %d\n", exit_status, term_signal);
    uv_close((uv_handle_t*)req, NULL);
}