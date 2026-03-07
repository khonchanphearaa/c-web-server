#ifndef HTTP_H
#define HTTP_H

#include "server.h"

/* Parse raw buffer into HttpRequest struct */
int  http_parse_request(const char *raw, HttpRequest *req);

/* Build and send HTTP response */
void http_send_response(int client_fd, HttpResponse *res);

/* Convenience: send a plain text or HTML response */
void http_send_text(int client_fd, HttpStatus status, const char *body);
void http_send_html(int client_fd, HttpStatus status, const char *body);
void http_send_json(int client_fd, HttpStatus status, const char *json);

/* Get the string label for a status code */
const char *http_status_str(HttpStatus status);

/* Get Content-Type from file extension */
const char *http_content_type(const char *path);

/* Free request resources */
void http_request_free(HttpRequest *req);

#endif 