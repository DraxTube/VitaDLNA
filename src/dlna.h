#ifndef DLNA_H
#define DLNA_H

#include <psp2/types.h>

#define DLNA_PORT 50000
#define SSDP_PORT 1900
#define SSDP_MULTICAST "239.255.255.250"
#define MAX_BUFFER 4096
#define MAX_URL_LENGTH 1024

typedef struct {
    char ip_address[16];
    int port;
    int is_running;
    char current_video_url[MAX_URL_LENGTH];
    int is_playing;
} DLNAContext;

// Inizializza il server DLNA
int dlna_init(DLNAContext *ctx);

// Avvia il server DLNA
int dlna_start(DLNAContext *ctx);

// Ferma il server DLNA
void dlna_stop(DLNAContext *ctx);

// Termina il server DLNA
void dlna_cleanup(DLNAContext *ctx);

// Ottieni lo stato corrente
int dlna_get_status(DLNAContext *ctx);

// Imposta URL video da riprodurre
int dlna_set_video_url(DLNAContext *ctx, const char *url);

#endif
