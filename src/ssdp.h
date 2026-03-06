#ifndef SSDP_H
#define SSDP_H

#include "dlna.h"

#define SSDP_NOTIFY_INTERVAL 30
#define SSDP_SEARCH_TIMEOUT 5

typedef struct {
    int socket_fd;
    struct sockaddr_in multicast_addr;
    struct sockaddr_in local_addr;
    int is_running;
} SSDPContext;

// Inizializza SSDP
int ssdp_init(SSDPContext *ctx, const char *ip_address);

// Avvia ascolto SSDP
int ssdp_start(SSDPContext *ctx);

// Invia risposta M-SEARCH
int ssdp_send_response(SSDPContext *ctx, int client_socket, struct sockaddr_in *client_addr);

// Invia notifica di presenza
int ssdp_send_notify(SSDPContext *ctx);

// Ferma SSDP
void ssdp_stop(SSDPContext *ctx);

// Pulisci SSDP
void ssdp_cleanup(SSDPContext *ctx);

// Ricevi e processa pacchetti SSDP
int ssdp_process_packet(SSDPContext *ctx, char *buffer, int buffer_size, struct sockaddr_in *from_addr);

#endif
