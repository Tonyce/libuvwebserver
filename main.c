#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

#define PORT 1234
#define DEFAULT_BACKLOG 511

uv_loop_t* loop;

typedef struct {
    uv_write_t write_req;
    uv_buf_t buf;
} client_t;

// 建立新的 tcp 连接后的回调
void new_connection(uv_stream_t* server, int status);
// libuv 将把 tcp 的数据放到 buf 上的回调
void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
// 读完 tcp 数据的回调，这里的 buf 将是 alloc_cb 的 buf，待会 debug 看下是不是。
void read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);
// libuv 写回 tcp 数据后的回调
void write_cb(uv_write_t* req, int status);

int main(int args, char* argv[]) {
    loop = uv_default_loop();
    struct sockaddr_in addr;              // tcp server addr will be bind
    uv_ip4_addr("0.0.0.0", PORT, &addr);  // init addr

    uv_tcp_t server;             // tcp server struct
    uv_tcp_init(loop, &server);  // init server with uv_tcp_init method

    uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);  // bind server and addr

    int result = uv_listen((uv_stream_t*)&server, DEFAULT_BACKLOG, new_connection);  // start server
    if (result) {
        fprintf(stderr, "Listen err %s\n", uv_strerror(result));
        return 1;
    }

    return uv_run(loop, UV_RUN_DEFAULT);
}

void new_connection(uv_stream_t* server, int status) {
    if (status < 0) {
        fprintf(stderr, "New connection error %s\n", uv_strerror(status));
        return;
    }

    client_t* client = (client_t*)malloc(sizeof(client_t));
    uv_tcp_init(loop, (uv_tcp_t*)client);

    if (uv_accept(server, (uv_stream_t*)client) == 0) {
        uv_read_start((uv_stream_t*)client, alloc_cb, read_cb);
    } else {
        uv_close((uv_handle_t*)client, NULL);
    }
}

void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    fprintf(stderr, "alloc cb %p\n", buf);
    buf->base = (char*)malloc(suggested_size);
    buf->len = suggested_size;
}

// read_cb 的 (const uv_buf_t* buf) point to (uv_buf_t* buf) in alloc_cb
void read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
    fprintf(stderr, "read cb %p\n", buf);
    if (nread >= 0) {
        client_t* client = (client_t*)malloc(sizeof(client_t));
        client->buf = uv_buf_init(buf->base, nread);
        uv_write((uv_write_t*)client, stream, &client->buf, 1, write_cb);
    } else {
        if (nread != UV_EOF) {
            fprintf(stderr, "Read error %s\n", uv_strerror(nread));
        }
    }
    fprintf(stderr, "read buf->base: %p\n", buf->base);
    // free(buf->base);
}

void write_cb(uv_write_t* req, int status) {
    if (status < 0) {
        fprintf(stderr, "Write error %s\n", uv_strerror(status));
    }
    client_t* client = (client_t*)req;
    fprintf(stderr, "write buf->base: %p\n", client->buf.base);
    free(client->buf.base);  // 74 行已经 free
    free(client);
};
