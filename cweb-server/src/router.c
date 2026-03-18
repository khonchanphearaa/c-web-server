#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/router.h"
#include "../include/http.h"
#include "../include/utils.h"
#include "../include/logger.h"


Router *router_create(void)
{
    Router *r = calloc(1, sizeof(Router));
    return r;
}

void router_add(Router *r, HttpMethod method, const char *path, RouteHandler handler)
{
    if (!r || r->count >= MAX_ROUTES)
        return;
    r->routes[r->count].method = method;
    r->routes[r->count].handler = handler;
    strncpy(r->routes[r->count].path, path, sizeof(r->routes[0].path) - 1);
    r->count++;
}

void router_destroy(Router *r)
{
    free(r);
}


/* Dispatch Request */
void router_dispatch(Router *r, int client_fd, HttpRequest *req)
{
    if (!r || !req)
        return;

    int path_matched = 0;

    for (int i = 0; i < r->count; i++)
    {
        if (strcmp(r->routes[i].path, req->path) == 0)
        {
            path_matched = 1;
            if (r->routes[i].method == req->method)
            {
                r->routes[i].handler(client_fd, req);
                return;
            }
        }
    }

    if (path_matched)
    {
        handler_method_not_allowed(client_fd, req);
    }
    else
    {
        handler_not_found(client_fd, req);
    }
}

/* Built-in Handlers */
void handler_not_found(int client_fd, HttpRequest *req)
{
    (void)req;
    http_send_html(client_fd, STATUS_404_NOT_FOUND,
                   "<!DOCTYPE html><html><head><title>404</title>"
                   "<style>body{font-family:sans-serif;background:#0f0f1a;color:#eee;"
                   "display:flex;justify-content:center;align-items:center;height:100vh;margin:0}"
                   ".box{background:#1a1a2e;padding:40px 60px;border-radius:12px;text-align:center}"
                   "h1{color:#e94560;font-size:4rem;margin:0}p{color:#a8b2d8}"
                   "a{color:#e94560;text-decoration:none}</style></head>"
                   "<body><div class='box'>"
                   "<h1>404</h1><p>Page not found.</p>"
                   "<p><a href='/'>&#8592; Go Home</a></p>"
                   "</div></body></html>");
}

void handler_method_not_allowed(int client_fd, HttpRequest *req)
{
    (void)req;
    http_send_json(client_fd, STATUS_405_METHOD_NOT_ALLOWED,
                   "{\"error\":\"Method Not Allowed\"}");
}

void handler_static_file(int client_fd, HttpRequest *req)
{
    /* req->path is like /static/css/style.css */
    /* Map to: static/css/style.css */
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s", req->path + 1); // strip leading /

    size_t file_len = 0;
    char *data = file_read(filepath, &file_len);

    if (!data)
    {
        handler_not_found(client_fd, req);
        return;
    }

    HttpResponse res = {0};
    res.status = STATUS_200_OK;
    res.body = data;
    res.body_len = file_len;
    strncpy(res.headers[0].key, "Content-Type", 127);
    strncpy(res.headers[0].value, http_content_type(filepath), 255);
    strncpy(res.headers[1].key, "Cache-Control", 127);
    strncpy(res.headers[1].value, "public, max-age=3600", 255);
    res.header_count = 2;

    http_send_response(client_fd, &res);
    free(data);
}