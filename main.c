// 系统
#include <stdio.h>
#include <stdlib.h>

// 第三方
#include <llhttp.h>
#include <uv.h>

#define PORT 1234
#define DEFAULT_BACKLOG 511

uv_loop_t* loop;
llhttp_settings_t settings;

const char* response =
    "HTTP/1.1 200 OK\r\n"
    "Date: Fri, 22 May 2009 06:07:21 GMT\r\n"
    "Content-Length: 10\r\n"
    "Content-Type: text/html; charset=UTF-8\r\n"
    "\r\n"
    "helloworld";

typedef struct {
    uv_tcp_t handle;       // libuv tcp
    uv_write_t write_req;  // write req to libuv
    llhttp_t parser;       // http parser
    uv_buf_t buf;          // data res
    char* url;
    char* body;
} client_t;

// 建立新的 tcp 连接后的回调
void new_connection(uv_stream_t* server, int status);
// libuv 将把 tcp 的数据放到 buf 上的回调
void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
// 读完 tcp 数据的回调，这里的 buf 将是 alloc_cb 的 buf，待会 debug 看下是不是。
void read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);
// libuv 写回 tcp 数据后的回调
void write_cb(uv_write_t* req, int status);

int handle_on_message_complete(llhttp_t* parser);
int on_body(llhttp_t* parser, const char* at, size_t body_len);
int on_url(llhttp_t* parser, const char* at, size_t url_len);

int main(int args, char* argv[]) {
    // 初始化 http parser settings
    llhttp_settings_init(&settings);
    /* Set user callback */
    settings.on_message_complete = handle_on_message_complete;
    settings.on_body = on_body;
    settings.on_url = on_url;

    loop = uv_default_loop();
    struct sockaddr_in addr;              // tcp server addr will be bind
    uv_ip4_addr("0.0.0.0", PORT, &addr);  // init addr

    uv_tcp_t server;             // tcp server struct
    uv_tcp_init(loop, &server);  // init server with uv_tcp_init method

    // fprintf(stderr, "main server: %p\n", &server);

    uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);  // bind server and addr

    int result = uv_listen((uv_stream_t*)&server, DEFAULT_BACKLOG, new_connection);  // start server
    if (result) {
        fprintf(stderr, "Listen err %s\n", uv_strerror(result));
        return 1;
    }

    return uv_run(loop, UV_RUN_DEFAULT);
}

void new_connection(uv_stream_t* server, int status) {
    // fprintf(stderr, "new_connection server: %p\n", server);
    if (status < 0) {
        fprintf(stderr, "New connection error %s\n", uv_strerror(status));
        return;
    }

    client_t* client = (client_t*)malloc(sizeof(client_t));
    uv_tcp_init(loop, (uv_tcp_t*)&client->handle);
    llhttp_init(&client->parser, HTTP_BOTH, &settings);

    // 要让 parser 也要持有 client，以供 parser cb 上下文取值
    client->parser.data = client;
    // 要让 handle 也要持有 client，以供 libuv cb 上下文取值
    client->handle.data = client;

    if (uv_accept(server, (uv_stream_t*)client) == 0) {
        uv_read_start((uv_stream_t*)&client->handle, alloc_cb, read_cb);
    } else {
        uv_close((uv_handle_t*)client, NULL);
    }
}

void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    // fprintf(stderr, "alloc cb %p\n", buf);
    buf->base = (char*)malloc(suggested_size);
    buf->len = suggested_size;
}

// read_cb 的 (const uv_buf_t* buf) point to (uv_buf_t* buf) in alloc_cb
void read_cb(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf) {
    // fprintf(stderr, "read cb %p\n", buf);
    if (nread >= 0) {
        client_t* client = (client_t*)handle->data;
        // parse data
        enum llhttp_errno err = llhttp_execute(&client->parser, buf->base, nread);
        if (err == HPE_OK) {
            /* Successfully parsed! */
        } else {
            fprintf(stderr, "Parse error: %s %s\n", llhttp_errno_name(err),
                    client->parser.reason);
        }

    } else {
        if (nread != UV_EOF) {
            fprintf(stderr, "Read error %s\n", uv_strerror(nread));
        }
    }
    // fprintf(stderr, "read buf->base: %p\n", buf->base);
    free(buf->base);
}

void write_cb(uv_write_t* req, int status) {
    if (status < 0) {
        fprintf(stderr, "Write error %s\n", uv_strerror(status));
    }
    client_t* client = (client_t*)req->data;
    free(client);
};

int handle_on_message_complete(llhttp_t* parser) {
    fprintf(stderr, "handle_on_message_complete\n");
    client_t* client = (client_t*)parser->data;

    fprintf(stderr, "method: %d\n", parser->method);
    fprintf(stderr, "url: %s\n", client->url);
    fprintf(stderr, "body: %s\n", client->body);
    // 这里就可以根据 url body 去做不同的事情
    // do other
    // 根据其它调用的结果响应不同的数据
    client->buf = uv_buf_init((char*)response, strlen(response));
    uv_write(
        (uv_write_t*)&client->write_req,
        (uv_stream_t*)&client->handle,
        &client->buf,
        1,
        write_cb);
    return 0;
};

// 这里的 url　是指在 url处的指针，所以要根据 len 去读取
int on_url(llhttp_t* parser, const char* at, size_t url_len) {
    char* url = NULL;
    url = (char*)malloc(url_len + 1);
    strncpy(url, at, url_len);
    // fprintf(stderr, "url: %s\nurl_len: %zu\n", url, url_len);

    client_t* client = (client_t*)parser->data;
    client->url = url;
    return 0;
};

// 这里的 body 是指在 body 处的指针，要根据 len 去读取
int on_body(llhttp_t* parser, const char* at, size_t body_len) {
    char* body = (char*)malloc(body_len + 1);
    strncpy(body, at, body_len);

    client_t* client = (client_t*)parser->data;
    client->body = body;

    return 0;
};