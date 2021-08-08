#ifndef __parser_h
#define __parser_h

#include <llhttp.h>

void init_parser_settings();
int on_body(llhttp_t* parser, const char* at, size_t body_len);
int on_url(llhttp_t* parser, const char* at, size_t url_len);

#endif