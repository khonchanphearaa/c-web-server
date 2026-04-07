#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "../include/server.h"
#include "../include/http.h"
#include "../include/router.h"
#include "../include/logger.h"
#include "../include/utils.h"
#include "../include/handlers.h"


/*  Server Lifecycle */
static Router *g_router = NULL;

static void handle_signal(int sig)
{
    (void)sig;
    LOG_INFO("Shutting down server...");
    if (g_router)
        router_destroy(g_router);
    logger_close();
    exit(0);
}

Server *server_create(int port)
{
    Server *srv = calloc(1, sizeof(Server));
    if (!srv)
        return NULL;

    srv->port = port;

    /* Create TCP socket */
    srv->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (srv->server_fd < 0)
    {
        perror("socket()");
        free(srv);
        return NULL;
    }

    /* Reuse address/port */
    int opt = 1;
    setsockopt(srv->server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    /* Bind */
    srv->address.sin_family = AF_INET;
    srv->address.sin_addr.s_addr = INADDR_ANY;
    srv->address.sin_port = htons(port);

    if (bind(srv->server_fd, (struct sockaddr *)&srv->address, sizeof(srv->address)) < 0)
    {
        perror("bind()");
        close(srv->server_fd);
        free(srv);
        return NULL;
    }

    /* Listen */
    if (listen(srv->server_fd, SERVER_BACKLOG) < 0)
    {
        perror("listen()");
        close(srv->server_fd);
        free(srv);
        return NULL;
    }

    srv->running = 1;
    return srv;
}

void server_run(Server *srv)
{
    /* Setup signal handler for clean shutdown */
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    /* Build router */
    g_router = router_create();
    router_add(g_router, METHOD_GET, "/", handle_home);
    // router_add(g_router, METHOD_GET, "/api/hello", handle_api_hello);
    // router_add(g_router, METHOD_GET, "/api/echo", handle_api_echo);
    // router_add(g_router, METHOD_POST, "/api/echo", handle_api_echo);
    // router_add(g_router, METHOD_GET,  "/api/info",  handle_api_info);
    // router_add(g_router, METHOD_GET,  "/api/books", handle_api_books);
    LOG_INFO("Server listening on http://0.0.0.0:%d", srv->port);
    printf("\n");
    printf("  ╔══════════════════════════════════════╗\n");
    printf("  ║       C Web Server v1.0.0            ║\n");
    printf("  ║  http://localhost:%-5d               ║\n", srv->port);
    printf("  ║  Press Ctrl+C to stop                ║\n");
    printf("  ╚══════════════════════════════════════╝\n\n");

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    char client_ip[INET_ADDRSTRLEN];

    while (srv->running)
    {
        int client_fd = accept(srv->server_fd,
                               (struct sockaddr *)&client_addr,
                               &client_len);
        if (client_fd < 0)
            continue;

        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));

        /* Read request */
        memset(buffer, 0, sizeof(buffer));
        ssize_t n = read(client_fd, buffer, sizeof(buffer) - 1);
        if (n <= 0)
        {
            close(client_fd);
            continue;
        }

        /* Parse request */
        HttpRequest req;
        memset(&req, 0, sizeof(req));
        if (http_parse_request(buffer, &req) < 0)
        {
            http_send_text(client_fd, STATUS_400_BAD_REQUEST, "400 Bad Request");
            close(client_fd);
            continue;
        }

        /* Log */
        LOG_INFO("[%s] %s %s",
                 client_ip,
                 req.method == METHOD_GET ? "GET" : req.method == METHOD_POST ? "POST"
                                                : req.method == METHOD_PUT    ? "PUT"
                                                : req.method == METHOD_DELETE ? "DELETE"
                                                                              : "?",
                 req.path);

        /* Serve static files for /static/ prefix */
        if (str_starts_with(req.path, "/static/"))
        {
            handler_static_file(client_fd, &req);
        }
        else
        {
            router_dispatch(g_router, client_fd, &req);
        }

        http_request_free(&req);
        close(client_fd);
    }
}

void server_destroy(Server *srv)
{
    if (!srv)
        return;
    close(srv->server_fd);
    free(srv);
}