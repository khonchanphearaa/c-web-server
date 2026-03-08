#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/http.h"
#include "../include/utils.h"


/* Status String */
const char *http_status_str(HttpStatus status)
{
    switch (status)
    {
    case STATUS_200_OK:
        return "200 OK";
    case STATUS_201_CREATED:
        return "201 Created";
    case STATUS_400_BAD_REQUEST:
        return "400 Bad Request";
    case STATUS_404_NOT_FOUND:
        return "404 Not Found";
    case STATUS_405_METHOD_NOT_ALLOWED:
        return "405 Method Not Allowed";
    case STATUS_500_INTERNAL_ERR:
        return "500 Internal Server Error";
    default:
        return "200 OK";
    }
}


/* Content-Type from extension */
const char *http_content_type(const char *path)
{
    const char *ext = strrchr(path, '.');
    if (!ext)
        return "application/octet-stream";
    if (strcmp(ext, ".html") == 0)
        return "text/html; charset=utf-8";
    if (strcmp(ext, ".css") == 0)
        return "text/css";
    if (strcmp(ext, ".js") == 0)
        return "application/javascript";
    if (strcmp(ext, ".json") == 0)
        return "application/json";
    if (strcmp(ext, ".png") == 0)
        return "image/png";
    if (strcmp(ext, ".jpg") == 0)
        return "image/jpeg";
    if (strcmp(ext, ".jpeg") == 0)
        return "image/jpeg";
    if (strcmp(ext, ".gif") == 0)
        return "image/gif";
    if (strcmp(ext, ".svg") == 0)
        return "image/svg+xml";
    if (strcmp(ext, ".ico") == 0)
        return "image/x-icon";
    if (strcmp(ext, ".txt") == 0)
        return "text/plain";
    return "application/octet-stream";
}

/* Parse HTTP Request */
int http_parse_request(const char *raw, HttpRequest *req)
{
    if (!raw || !req)
        return -1;

    char line[512];
    const char *p = raw;

    /* Parse request line: METHOD PATH PROTOCOL */
    int i = 0;
    while (*p && *p != '\n' && i < (int)sizeof(line) - 1)
        line[i++] = *p++;
    line[i] = '\0';
    if (*p == '\n')
        p++;

    char method_str[16] = {0}, path_full[512] = {0}, proto[16] = {0};
    sscanf(line, "%15s %511s %15s", method_str, path_full, proto);

    /* Method */
    if (strcmp(method_str, "GET") == 0)
        req->method = METHOD_GET;
    else if (strcmp(method_str, "POST") == 0)
        req->method = METHOD_POST;
    else if (strcmp(method_str, "PUT") == 0)
        req->method = METHOD_PUT;
    else if (strcmp(method_str, "DELETE") == 0)
        req->method = METHOD_DELETE;


    strncpy(req->protocol, proto, sizeof(req->protocol) - 1);

    /* Split path and query string */
    char *q = strchr(path_full, '?');
    if (q)
    {
        *q = '\0';
        url_decode(q + 1, req->query, sizeof(req->query));
    }
    url_decode(path_full, req->path, sizeof(req->path));

    /* Parse headers */
    req->header_count = 0;
    while (*p && req->header_count < MAX_HEADERS)
    {
        i = 0;
        while (*p && *p != '\n' && i < (int)sizeof(line) - 1)
            line[i++] = *p++;
        line[i] = '\0';
        if (*p == '\n')
            p++;

        /* Blank line = end of headers */
        str_trim(line);
        if (strlen(line) == 0)
            break;

        char *colon = strchr(line, ':');
        if (colon)
        {
            *colon = '\0';
            char *val = colon + 1;
            str_trim(line);
            str_trim(val);
            strncpy(req->headers[req->header_count].key, line, 127);
            strncpy(req->headers[req->header_count].value, val, 255);
            req->header_count++;
        }
    }

    /* Body (whatever remains) */
    if (*p)
    {
        size_t body_len = strlen(p);
        req->body = str_dup(p);
        req->body_len = body_len;
    }

    return 0;
}

/* Send Response */
void http_send_response(int client_fd, HttpResponse *res)
{
    char header_buf[4096];
    int off = 0;

    off += snprintf(header_buf + off, sizeof(header_buf) - off,
                    "HTTP/1.1 %s\r\n", http_status_str(res->status));

    for (int i = 0; i < res->header_count; i++)
    {
        off += snprintf(header_buf + off, sizeof(header_buf) - off,
                        "%s: %s\r\n",
                        res->headers[i].key, res->headers[i].value);
    }

    if (res->body_len > 0)
    {
        off += snprintf(header_buf + off, sizeof(header_buf) - off,
                        "Content-Length: %zu\r\n", res->body_len);
    }

    off += snprintf(header_buf + off, sizeof(header_buf) - off,
                    "Connection: close\r\n\r\n");

    write(client_fd, header_buf, off);
    if (res->body && res->body_len > 0)
        write(client_fd, res->body, res->body_len);
}


/* Convenience senders */
static void send_with_type(int client_fd, HttpStatus status,
                           const char *body, const char *ctype)
{
    HttpResponse res = {0};
    res.status = status;
    res.body = (char *)body;
    res.body_len = body ? strlen(body) : 0;
    strncpy(res.headers[0].key, "Content-Type", 127);
    strncpy(res.headers[0].value, ctype, 255);
    res.header_count = 1;
    http_send_response(client_fd, &res);
}

void http_send_text(int fd, HttpStatus s, const char *b)
{
    send_with_type(fd, s, b, "text/plain; charset=utf-8");
}

void http_send_html(int fd, HttpStatus s, const char *b)
{
    send_with_type(fd, s, b, "text/html; charset=utf-8");
}

void http_send_json(int fd, HttpStatus s, const char *b)
{
    send_with_type(fd, s, b, "application/json");
}


/* Cleanup */
void http_request_free(HttpRequest *req)
{
    if (req && req->body)
    {
        free(req->body);
        req->body = NULL;
    }
}