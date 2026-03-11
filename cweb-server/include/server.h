#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* Server Config */
#define SERVER_PORT 8080
#define SERVER_BACKLOG 10
#define BUFFER_SIZE 8192
#define MAX_HEADERS 32
#define STATIC_DIR "static"
#define LOG_FILE "logs/server.log"

/* HTTP methods */
typedef enum
{
    METHOD_GET,
    METHOD_POST,
    METHOD_PUT,
    METHOD_DELETE,
} HttpMethod;

/* HTTP status codes */
typedef enum
{
    STATUS_200_OK = 200,
    STATUS_201_CREATED = 201,
    STATUS_400_BAD_REQUEST = 400,
    STATUS_404_NOT_FOUND = 404,
    STATUS_405_METHOD_NOT_ALLOWED = 405,
    STATUS_500_INTERNAL_ERR = 500
} HttpStatus;

/* HTTP headers */
typedef struct
{
    char key[128];
    char value[256];
} HttpHeader;

/* HTTP request */
typedef struct
{
    HttpMethod method;
    char path[256];
    char query[512]; 
    char protocol[16];
    HttpHeader headers[MAX_HEADERS];
    int header_count;
    char *body;
    size_t body_len;
} HttpRequest;

/* HTTP response */
typedef struct
{
    HttpStatus status;
    HttpHeader headers[MAX_HEADERS];
    int header_count;
    char *body;
    size_t body_len;
} HttpResponse;

/* Server state */
typedef struct
{
    int server_fd;
    struct sockaddr_in address;
    int port;
    int running;
} Server;

/* Server functions */
Server *server_create(int port);
void server_run(Server *srv);
void server_destroy(Server *srv);

#endif