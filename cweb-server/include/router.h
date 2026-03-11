#ifndef ROUTER_H
#define ROUTER_H

#include "server.h"

#define MAX_ROUTES 64

/* Handler function signature: receives request, writes response to client_fd */
typedef void (*RouteHandler)(int client_fd, HttpRequest *req);

/* A single route entry */
typedef struct
{
    HttpMethod method;
    char path[256];
    RouteHandler handler;
} Route;

/* Router holds all registered routes */
typedef struct
{
    Route routes[MAX_ROUTES];
    int count;
} Router;

/* Router API */
Router *router_create(void);
void router_add(Router *r, HttpMethod method, const char *path, RouteHandler handler);
void router_dispatch(Router *r, int client_fd, HttpRequest *req);
void router_destroy(Router *r);

/* Built-in Handlers */
void handler_static_file(int client_fd, HttpRequest *req);
void handler_not_found(int client_fd, HttpRequest *req);
void handler_method_not_allowed(int client_fd, HttpRequest *req);

#endif