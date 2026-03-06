#include <psp2/kernel/processmgr.h>
#include <psp2/net/net.h>
#include <psp2/net/netctl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ssdp.h"
#include "network.h"
#include "utils.h"

static const char *SSDP_RESPONSE_TEMPLATE = 
    "HTTP/1.1 200 OK\r\n"
    "CACHE-CONTROL: max-age=1800\r\n"
    "EXT: \r\n"
    "LOCATION: http://%s:%d/description.xml\r\n"
    "SERVER: PS Vita DLNA/1.0\r\n"
    "ST: urn:schemas-upnp-org:device:MediaRenderer:1\r\n"
    "USN: uuid:VitaDLNA-001::urn:schemas-upnp-org:device:MediaRenderer:1\r\n"
    "\r\n";

static const char *SSDP_NOTIFY_TEMPLATE = 
    "NOTIFY * HTTP/1.1\r\n"
    "HOST: 239.255.255.250:1900\r\n"
    "CACHE-CONTROL: max-age=1800\r\n"
    "LOCATION: http://%s:%d/description.xml\r\n"
    "NT: urn:schemas-upnp-org:device:MediaRenderer:1\r\n"
    "NTS: ssdp:alive\r\n"
    "SERVER: PS Vita DLNA/1.0\r\n"
    "USN: uuid:VitaDLNA-001::urn:schemas-upnp-org:device:MediaRenderer:1\r\n"
    "\r\n";

int ssdp_init(SSDPContext *ctx, const char *ip_address) {
    if (!ctx || !ip_address) return -1;
    
    memset(ctx, 0, sizeof(SSDPContext));
    
    // Crea socket UDP
    ctx->socket_fd = network_create_udp_socket();
    if (ctx->socket_fd < 0) {
        debug_print("Errore creazione socket SSDP");
        return -1;
    }
    
    // Configura indirizzo multicast
    memset(&ctx->multicast_addr, 0, sizeof(ctx->multicast_addr));
    ctx->multicast_addr.sin_family = SCE_NET_AF_INET;
    ctx->multicast_addr.sin_port = sceNetHtons(SSDP_PORT);
    // sceNetInetPton(SCE_NET_AF_INET, SSDP_MULTICAST, &ctx->multicast_addr.sin_addr);
    
    // Configura indirizzo locale
    memset(&ctx->local_addr, 0, sizeof(ctx->local_addr));
    ctx->local_addr.sin_family = SCE_NET_AF_INET;
    ctx->local_addr.sin_port = sceNetHtons(SSDP_PORT);
    // sceNetInetPton(SCE_NET_AF_INET, ip_address, &ctx->local_addr.sin_addr);
    
    ctx->is_running = 0;
    
    debug_print("SSDP Context inizializzato");
    return 0;
}

int ssdp_start(SSDPContext *ctx) {
    if (!ctx) return -1;
    
    // Bind socket
    int ret = network_bind_socket(ctx->socket_fd, "0.0.0.0", SSDP_PORT);
    if (ret < 0) {
        debug_print("Errore bind socket SSDP: %d", ret);
        return -1;
    }
    
    ctx->is_running = 1;
    debug_print("SSDP in ascolto su porta %d", SSDP_PORT);
    
    return 0;
}

int ssdp_send_response(SSDPContext *ctx, int client_socket, struct sockaddr_in *client_addr) {
    if (!ctx || client_socket < 0) return -1;
    
    char response[MAX_BUFFER];
    snprintf(response, sizeof(response), SSDP_RESPONSE_TEMPLATE, 
             "192.168.1.100", HTTP_PORT); // IP da configurare dinamicamente
    
    int ret = network_send_data(client_socket, response, strlen(response));
    if (ret < 0) {
        debug_print("Errore invio risposta SSDP");
        return -1;
    }
    
    debug_print("Risposta SSDP inviata");
    return 0;
}

int ssdp_send_notify(SSDPContext *ctx) {
    if (!ctx || !ctx->is_running) return -1;
    
    char notify[MAX_BUFFER];
    snprintf(notify, sizeof(notify), SSDP_NOTIFY_TEMPLATE,
             "192.168.1.100", HTTP_PORT); // IP da configurare dinamicamente
    
    // Invia al multicast
    int ret = network_send_data(ctx->socket_fd, notify, strlen(notify));
    if (ret < 0) {
        debug_print("Errore invio notifica SSDP");
        return -1;
    }
    
    debug_print("Notifica SSDP inviata");
    return 0;
}

void ssdp_stop(SSDPContext *ctx) {
    if (!ctx || !ctx->is_running) return;
    
    ctx->is_running = 0;
    debug_print("SSDP arrestato");
}

void ssdp_cleanup(SSDPContext *ctx) {
    if (!ctx) return;
    
    if (ctx->socket_fd >= 0) {
        network_close_socket(ctx->socket_fd);
    }
    
    debug_print("SSDP Context pulito");
}

int ssdp_process_packet(SSDPContext *ctx, char *buffer, int buffer_size, struct sockaddr_in *from_addr) {
    if (!ctx || !buffer || buffer_size <= 0) return -1;
    
    // Cerca "M-SEARCH" nella richiesta
    if (strstr(buffer, "M-SEARCH")) {
        debug_print("Ricevuta richiesta M-SEARCH");
        
        // Crea socket per risposta
        int response_socket = network_create_tcp_socket();
        if (response_socket >= 0) {
            ssdp_send_response(ctx, response_socket, from_addr);
            network_close_socket(response_socket);
        }
        
        return 1;
    }
    
    return 0;
}
