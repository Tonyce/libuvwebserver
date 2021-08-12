#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
//
#include <uv.h>

uv_loop_t* loop;

void on_new_connection(uv_stream_t* server, int status);
void alloc_buf(uv_handle_t* handle,
               size_t suggested_size,
               uv_buf_t* buf);
void echo_read(uv_stream_t* stream,
               ssize_t nread,
               const uv_buf_t* buf);

int main(int argc, char* argv[]) {
    loop = uv_default_loop();

    struct sockaddr_in addr;
    uv_ip4_addr("0.0.0.0", 1234, &addr);

    uv_tcp_t server;
    uv_tcp_init(loop, &server);

    uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);
    int r = uv_listen((uv_stream_t*)&server, 511, on_new_connection);
    if (r) {
        fprintf(stderr, "listen error: %s\n", uv_strerror(r));
        return 1;
    }

    return uv_run(loop, UV_RUN_DEFAULT);
}

void on_new_connection(uv_stream_t* server, int status) {
    if (status < 0) {
        fprintf(stderr, "on_new_connection error: %s\n", uv_strerror(status));
        return;
    }
    // uv_tcp_t 是 handle，也是 stream
    uv_tcp_t* client = malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, client);

    if (uv_accept(server, (uv_stream_t*)client) == 0) {
        uv_read_start((uv_stream_t*)client, alloc_buf, echo_read);
    } else {
        uv_close((uv_handle_t*)client, NULL);
    }
};

void alloc_buf(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    *buf = uv_buf_init((char*)malloc(suggested_size), suggested_size);
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
        // write_req_t* req = (write_req_t*)malloc(sizeof(write_req_t));
        // req->buf = uv_buf_init(buf->base, nread);
        // uv_write((uv_write_t*)&req->req, client, &req->buf, 1, echo_write);
    }
    free(buf->base);
};