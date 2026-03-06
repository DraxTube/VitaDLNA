#include <psp2/kernel/processmgr.h>
#include <psp2/sysmodule.h>
#include <psp2/net/net.h>
#include <psp2/net/netctl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "dlna.h"
#include "ssdp.h"
#include "soap_server.h"
#include "http_server.h"
#include "network.h"
#include "utils.h"

static SSDPContext g_ssdp_ctx;
static SOAPContext g_soap_ctx;
static HTTPContext g_http_ctx;

int dlna_init(DLNAContext *ctx) {
    if (!ctx) return -1;
    
    memset(ctx, 0, sizeof(DLNAContext));
    ctx->port = DLNA_PORT;
    ctx->is_running = 0;
    ctx->is_playing = 0;
    ctx->current_video_url[0] = '\0';
    
    debug_print("DLNA Context inizializzato");
    return 0;
}

int dlna_start(DLNAContext *ctx) {
    int ret;
    
    if (!ctx || ctx->is_running) return -1;
    
    debug_print("Avvio server DLNA su %s:%d", ctx->ip_address, ctx->port);
    
    // Inizializza SSDP (discovery)
    ret = ssdp_init(&g_ssdp_ctx, ctx->ip_address);
    if (ret < 0) {
        debug_print("Errore init SSDP: %d", ret);
        return -1;
    }
    
    ret = ssdp_start(&g_ssdp_ctx);
    if (ret < 0) {
        debug_print("Errore start SSDP: %d", ret);
        return -1;
    }
    
    debug_print("SSDP avviato (porta %d)", SSDP_PORT);
    
    // Inizializza SOAP (controllo)
    ret = soap_init(&g_soap_ctx, ctx, ctx->ip_address);
    if (ret < 0) {
        debug_print("Errore init SOAP: %d", ret);
        ssdp_cleanup(&g_ssdp_ctx);
        return -1;
    }
    
    ret = soap_start(&g_soap_ctx);
    if (ret < 0) {
        debug_print("Errore start SOAP: %d", ret);
        ssdp_cleanup(&g_ssdp_ctx);
        return -1;
    }
    
    debug_print("SOAP avviato (porta %d)", SOAP_PORT);
    
    // Inizializza HTTP (descrizione dispositivo)
    ret = http_init(&g_http_ctx, ctx, ctx->ip_address);
    if (ret < 0) {
        debug_print("Errore init HTTP: %d", ret);
        ssdp_cleanup(&g_ssdp_ctx);
        soap_cleanup(&g_soap_ctx);
        return -1;
    }
    
    ret = http_start(&g_http_ctx);
    if (ret < 0) {
        debug_print("Errore start HTTP: %d", ret);
        ssdp_cleanup(&g_ssdp_ctx);
        soap_cleanup(&g_soap_ctx);
        return -1;
    }
    
    debug_print("HTTP avviato (porta %d)", HTTP_PORT);
    
    ctx->is_running = 1;
    
    // Invia prima notifica di presenza
    ssdp_send_notify(&g_ssdp_ctx);
    
    debug_print("Server DLNA completamente avviato!");
    return 0;
}

void dlna_stop(DLNAContext *ctx) {
    if (!ctx || !ctx->is_running) return;
    
    debug_print("Arresto server DLNA...");
    
    ctx->is_running = 0;
    
    ssdp_stop(&g_ssdp_ctx);
    soap_stop(&g_soap_ctx);
    http_stop(&g_http_ctx);
    
    debug_print("Server DLNA arrestato");
}

void dlna_cleanup(DLNAContext *ctx) {
    if (!ctx) return;
    
    ssdp_cleanup(&g_ssdp_ctx);
    soap_cleanup(&g_soap_ctx);
    http_cleanup(&g_http_ctx);
    
    debug_print("DLNA Context pulito");
}

int dlna_get_status(DLNAContext *ctx) {
    if (!ctx) return -1;
    return ctx->is_running ? 1 : 0;
}

int dlna_set_video_url(DLNAContext *ctx, const char *url) {
    if (!ctx || !url) return -1;
    
    strncpy(ctx->current_video_url, url, MAX_URL_LENGTH - 1);
    ctx->current_video_url[MAX_URL_LENGTH - 1] = '\0';
    
    debug_print("URL video impostato: %s", ctx->current_video_url);
    return 0;
}
