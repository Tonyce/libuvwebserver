#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <uv.h>

uv_loop_t* loop;
uv_pipe_t queue;

void alloc_buf(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
void on_new_connection(uv_stream_t* req, ssize_t nread, const uv_buf_t* buf);
void echo_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);

int main(int argc, char* argv[]) {
    loop = uv_default_loop();

    uv_pipe_init(loop, &queue, 1 /* ipc */);
    uv_pipe_open(&queue, 0);  // 0代表stdin

    uv_read_start((uv_stream_t*)&queue, alloc_buf, on_new_connection);

    return uv_run(loop, UV_RUN_DEFAULT);
}

void alloc_buf(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    *buf = uv_buf_init((char*)malloc(suggested_size), suggested_size);
};

void on_new_connection(uv_stream_t* req, ssize_t nread, const uv_buf_t* buf) {
    if (nread < 0) {
        if (nread != UV_EOF) {
            fprintf(stderr, "on_new_connection error: %s\n", uv_strerror(nread));
        }
        uv_close((uv_handle_t*)req, NULL);
        return;
    }

    uv_pipe_t* pipe = (uv_pipe_t*)req;
    if (!uv_pipe_pending_count(pipe)) {
        fprintf(stderr, "No pending count\n");
        return;
    }

    uv_handle_type pending = uv_pipe_pending_type(pipe);
    assert(pending == UV_TCP);

    uv_tcp_t* client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, client);

    if (uv_accept(req, (uv_stream_t*)client) == 0) {
        uv_os_fd_t fd;
        uv_fileno((const uv_handle_t*)client, &fd);
        fprintf(stderr, "Worker %d: Accepted fd %d\n", getpid(), fd);
        uv_read_start((uv_stream_t*)client, alloc_buf, echo_read);
    } else {
        uv_close((uv_handle_t*)client, NULL);
    }
};

// write_req 就是 echo_read write_req
void echo_write(uv_write_t* write_req, int status) {
    if (status) {
        fprintf(stderr, "Write error %s\n", uv_strerror(status));
    }
    free(write_req);
}

void echo_read(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf) {
    if (nread < 0) {
        if (nread != UV_EOF) {
            fprintf(stderr, "echo_read error: %s\n", uv_strerror(nread));
        }
    } else {
        uv_write_t* write_req = (uv_write_t*)malloc(sizeof(uv_write_t));
        uv_buf_t write_buf = uv_buf_init(buf->base, nread);
        uv_write(write_req, client, &write_buf, 1, echo_write);
    }
    free(buf->base);
};