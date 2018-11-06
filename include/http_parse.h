#ifndef HTTP_PARSE_H
#define HTTP_PARSE_H

#include "http_request.h"
#include "http.h"

#define CR '\r'
#define LF '\n'

int wb_http_parse_request_line(wb_http_request_t *request);
int wb_http_parse_request_body(wb_http_request_t *request);

#endif //!HTTP_PARSE_H