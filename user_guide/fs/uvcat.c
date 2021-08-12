#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

//
#include <uv.h>

uv_fs_t open_req, write_req, read_req;

uv_loop_t* loop;
uv_buf_t iov;
static char buffer[1024];

void on_open(uv_fs_t* req);
void on_read(uv_fs_t* req);
void on_write(uv_fs_t* req);

int main(int argc, char* argv[]) {
    loop = uv_default_loop();
    uv_fs_open(loop, &open_req, argv[1], O_RDONLY, 0, on_open);
    uv_run(loop, UV_RUN_DEFAULT);

    uv_fs_req_cleanup(&open_req);
    uv_fs_req_cleanup(&read_req);
    uv_fs_req_cleanup(&write_req);
    return 0;
}

void on_open(uv_fs_t* req) {
    assert(req == &open_req);
    if (req->result < 0) {
        fprintf(stderr, "error opening file: %s\n", uv_strerror(req->result));
        return;
    }
    iov = uv_buf_init(buffer, sizeof(buffer));
    // uv_fs_t的result域保存了uv_fs_open回调函数打开的文件描述符。如果文件被正确地打开，我们可以开始读取了。
    uv_fs_read(uv_default_loop(), &read_req, open_req.result, &iov, 1, -1, on_read);
}

void on_write(uv_fs_t* req) {
    if (req->result < 0) {
        fprintf(stderr, "Write error: %s\n", uv_strerror(req->result));
        return;
    }
    uv_fs_read(uv_default_loop(), &read_req, open_req.result, &iov, 1, -1, on_read);
}

void on_read(uv_fs_t* req) {
    printf("on_read\n");
    if (req->result < 0) {
        fprintf(stderr, "Read error: %s\n", uv_strerror(req->result));
        return;
    }
    if (req->result == 0) {  // eof
        uv_fs_t close_req;
        uv_fs_close(uv_default_loop(), &close_req, open_req.result, NULL);
        return;
    }
    iov.len = req->result;
    int stdout = 1;
    uv_fs_write(uv_default_loop(), &write_req, stdout, &iov, 1, -1, on_write);
}
