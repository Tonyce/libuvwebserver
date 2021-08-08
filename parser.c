// 系统
#include <stdio.h>
#include <stdlib.h>

// 第三方
#include <llhttp.h>

#include "client.h"
#include "parser.h"

llhttp_settings_t settings;

void init_parser_settings() {
    // 初始化 http parser settings
    llhttp_settings_init(&settings);
    /* Set user callback */
    // settings.on_message_complete = handle_on_message_complete;
    settings.on_body = on_body;
    settings.on_url = on_url;
}

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