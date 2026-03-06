#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "dlna.h"

#define HTTP_PORT 50002

typedef struct {
    int server_socket;
    struct sockaddr_in server_addr;
    int is_running;
    DLNAContext *dlna_ctx;
} HTTPContext;

// Inizializza server HTTP
int http_init(HTTPContext *ctx, DLNAContext *dlna_ctx, const char *ip_address);

// Avvia server HTTP
int http_start(HTTPContext *ctx);

// Processa richiesta HTTP
int http_process_request(HTTPContext *ctx, int client_socket);

// Invia descrizione dispositivo XML
int http_send_device_description(HTTPContext *ctx, int client_socket);

// Invia risposta 404
int http_send_404(HTTPContext *ctx, int client_socket);

// Ferma server HTTP
void http_stop(HTTPContext *ctx);

// Pulisci server HTTP
void http_cleanup(HTTPContext *ctx);

#endif
