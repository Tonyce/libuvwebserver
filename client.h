#ifndef __client_h
#define __client_h

#include <llhttp.h>
#include <uv.h>

typedef struct {
    uv_tcp_t handle;       // libuv tcp
    uv_write_t write_req;  // write req to libuv
    llhttp_t parser;       // http parser
    uv_buf_t res;          // data res
    char* url;
    char* body;
} client_t;

#endif