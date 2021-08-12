#include <stdio.h>
#include <stdlib.h>

//
#include <uv.h>

uv_loop_t* loop;

void on_resolve(uv_getaddrinfo_t* req,
                int status,
                struct addrinfo* res);

int main(int argc, char* argv[]) {
    loop = uv_default_loop();

    struct addrinfo hints;
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = 0;

    uv_getaddrinfo_t addr_req;
    fprintf(stderr, "baidu.com is ...");
    int r = uv_getaddrinfo(loop, &addr_req, on_resolve, "baidu.com", "80", &hints);
    if (r) {
        fprintf(stderr, "getaddrinfo call error %s\n", uv_err_name(r));
        return 1;
    }
    return uv_run(loop, UV_RUN_DEFAULT);
}

void on_resolve(uv_getaddrinfo_t* req,
                int status,
                struct addrinfo* res) {
    char addr[17] = {'\0'};
    uv_ip4_name((const struct sockaddr_in*)res->ai_addr, addr, 16);
    fprintf(stderr, "%s\n", addr);
    uv_freeaddrinfo(res);
};