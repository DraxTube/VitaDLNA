#include <psp2/kernel/processmgr.h>
#include <psp2/net/net.h>
#include <psp2/sysmodule.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "soap_server.h"
#include "network.h"
#include "utils.h"
#include "video_player.h"

static const char *SOAP_ENVELOPE_START = 
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
    "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" "
    "s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n"
    "<s:Body>\r\n";

static const char *SOAP_ENVELOPE_END = 
    "</s:Body>\r\n"
    "</s:Envelope>\r\n";

static const char *SET_AV_TRANSPORT_RESPONSE = 
    "<u:SetAVTransportURIResponse xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\"/>\r\n";

static const char *PLAY_RESPONSE = 
    "<u:PlayResponse xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\"/>\r\n";

static const char *STOP_RESPONSE = 
    "<u:StopResponse xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\"/>\r\n";

static const char *GET_POSITION_RESPONSE_TEMPLATE = 
    "<u:GetPositionInfoResponse xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\">\r\n"
    "<TrackDuration>0:00:00</TrackDuration>\r\n"
    "<RelTime>0:00:%d</RelTime>\r\n"
    "<AbsTime>0:00:%d</AbsTime>\r\n"
    "</u:GetPositionInfoResponse>\r\n";

int soap_init(SOAPContext *ctx, DLNAContext *dlna_ctx, const char *ip_address) {
    if (!ctx || !dlna_ctx || !ip_address) return -1;
    
    memset(ctx, 0, sizeof(SOAPContext));
    ctx->dlna_ctx = dlna_ctx;
    
    // Crea socket server
    ctx->server_socket = network_create_tcp_socket();
    if (ctx->server_socket < 0) {
        debug_print("Errore creazione socket SOAP");
        return -1;
    }
    
    // Configura indirizzo server
    memset(&ctx->server_addr, 0, sizeof(ctx->server_addr));
    ctx->server_addr.sin_family = SCE_NET_AF_INET;
    ctx->server_addr.sin_port = sceNetHtons(SOAP_PORT);
    // sceNetInetPton(SCE_NET_AF_INET, ip_address, &ctx->server_addr.sin_addr);
    
    ctx->client_socket = -1;
    ctx->is_running = 0;
    
    debug_print("SOAP Context inizializzato");
    return 0;
}

int soap_start(SOAPContext *ctx) {
    if (!ctx) return -1;
    
    // Bind socket
    int ret = network_bind_socket(ctx->server_socket, "0.0.0.0", SOAP_PORT);
    if (ret < 0) {
        debug_print("Errore bind socket SOAP: %d", ret);
        return -1;
    }
    
    // Listen
    ret = network_listen_socket(ctx->server_socket, 5);
    if (ret < 0) {
        debug_print("Errore listen socket SOAP: %d", ret);
        return -1;
    }
    
    ctx->is_running = 1;
    debug_print("SOAP server in ascolto su porta %d", SOAP_PORT);
    
    return 0;
}

int soap_process_request(SOAPContext *ctx, char *request, int request_size) {
    if (!ctx || !request || request_size <= 0) return -1;
    
    char action[256];
    char url[MAX_URL_LENGTH];
    
    // Estrai azione dalla richiesta
    if (extract_action_from_soap(request, action, sizeof(action)) < 0) {
        debug_print("Impossibile estrarre azione SOAP");
        return -1;
    }
    
    debug_print("Azione SOAP ricevuta: %s", action);
    
    // Gestisci diverse azioni
    if (strcmp(action, "SetAVTransportURI") == 0) {
        if (extract_url_from_soap(request, url, sizeof(url)) >= 0) {
            debug_print("URL ricevuto: %s", url);
            dlna_set_video_url(ctx->dlna_ctx, url);
            soap_handle_set_av_transport(ctx, url);
        }
    } else if (strcmp(action, "Play") == 0) {
        debug_print("Ricevuto comando Play");
        soap_handle_play(ctx);
    } else if (strcmp(action, "Stop") == 0) {
        debug_print("Ricevuto comando Stop");
        soap_handle_stop(ctx);
    } else if (strcmp(action, "GetPositionInfo") == 0) {
        soap_handle_get_position(ctx);
    } else {
        debug_print("Azione non supportata: %s", action);
    }
    
    return 0;
}

int soap_handle_set_av_transport(SOAPContext *ctx, const char *uri) {
    if (!ctx || !uri) return -1;
    
    char response[MAX_BUFFER];
    snprintf(response, sizeof(response), 
             "HTTP/1.1 200 OK\r\n"
             "CONTENT-TYPE: text/xml; charset=\"utf-8\"\r\n"
             "CONTENT-LENGTH: %d\r\n"
             "SERVER: PS Vita DLNA/1.0\r\n"
             "CONNECTION: close\r\n"
             "\r\n"
             "%s%s%s",
             (int)(strlen(SOAP_ENVELOPE_START) + strlen(SET_AV_TRANSPORT_RESPONSE) + strlen(SOAP_ENVELOPE_END)),
             SOAP_ENVELOPE_START,
             SET_AV_TRANSPORT_RESPONSE,
             SOAP_ENVELOPE_END);
    
    network_send_data(ctx->client_socket, response, strlen(response));
    debug_print("Risposta SetAVTransportURI inviata");
    
    return 0;
}

int soap_handle_play(SOAPContext *ctx) {
    if (!ctx) return -1;
    
    // Avvia riproduzione video
    if (ctx->dlna_ctx && ctx->dlna_ctx->current_video_url[0] != '\0') {
        // video_player_play(&g_player_ctx, ctx->dlna_ctx->current_video_url);
        debug_print("Riproduzione avviata (simulata)");
    }
    
    char response[MAX_BUFFER];
    snprintf(response, sizeof(response),
             "HTTP/1.1 200 OK\r\n"
             "CONTENT-TYPE: text/xml; charset=\"utf-8\"\r\n"
             "CONTENT-LENGTH: %d\r\n"
             "SERVER: PS Vita DLNA/1.0\r\n"
             "CONNECTION: close\r\n"
             "\r\n"
             "%s%s%s",
             (int)(strlen(SOAP_ENVELOPE_START) + strlen(PLAY_RESPONSE) + strlen(SOAP_ENVELOPE_END)),
             SOAP_ENVELOPE_START,
             PLAY_RESPONSE,
             SOAP_ENVELOPE_END);
    
    network_send_data(ctx->client_socket, response, strlen(response));
    debug_print("Risposta Play inviata");
    
    return 0;
}

int soap_handle_stop(SOAPContext *ctx) {
    if (!ctx) return -1;
    
    // Ferma riproduzione video
    // video_player_stop(&g_player_ctx);
    debug_print("Riproduzione fermata (simulata)");
    
    char response[MAX_BUFFER];
    snprintf(response, sizeof(response),
             "HTTP/1.1 200 OK\r\n"
             "CONTENT-TYPE: text/xml; charset=\"utf-8\"\r\n"
             "CONTENT-LENGTH: %d\r\n"
             "SERVER: PS Vita DLNA/1.0\r\n"
             "CONNECTION: close\r\n"
             "\r\n"
             "%s%s%s",
             (int)(strlen(SOAP_ENVELOPE_START) + strlen(STOP_RESPONSE) + strlen(SOAP_ENVELOPE_END)),
             SOAP_ENVELOPE_START,
             STOP_RESPONSE,
             SOAP_ENVELOPE_END);
    
    network_send_data(ctx->client_socket, response, strlen(response));
    debug_print("Risposta Stop inviata");
    
    return 0;
}

int soap_handle_get_position(SOAPContext *ctx) {
    if (!ctx) return -1;
    
    int position_seconds = 0; // Da ottenere dal player reale
    
    char response[MAX_BUFFER];
    char position_response[512];
    snprintf(position_response, sizeof(position_response), GET_POSITION_RESPONSE_TEMPLATE,
             position_seconds, position_seconds);
    
    snprintf(response, sizeof(response),
             "HTTP/1.1 200 OK\r\n"
             "CONTENT-TYPE: text/xml; charset=\"utf-8\"\r\n"
             "CONTENT-LENGTH: %d\r\n"
             "SERVER: PS Vita DLNA/1.0\r\n"
             "CONNECTION: close\r\n"
             "\r\n"
             "%s%s%s",
             (int)(strlen(SOAP_ENVELOPE_START) + strlen(position_response) + strlen(SOAP_ENVELOPE_END)),
             SOAP_ENVELOPE_START,
             position_response,
             SOAP_ENVELOPE_END);
    
    network_send_data(ctx->client_socket, response, strlen(response));
    debug_print("Risposta GetPositionInfo inviata");
    
    return 0;
}

void soap_stop(SOAPContext *ctx) {
    if (!ctx || !ctx->is_running) return;
    
    ctx->is_running = 0;
    
    if (ctx->client_socket >= 0) {
        network_close_socket(ctx->client_socket);
        ctx->client_socket = -1;
    }
    
    debug_print("SOAP server arrestato");
}

void soap_cleanup(SOAPContext *ctx) {
    if (!ctx) return;
    
    if (ctx->server_socket >= 0) {
        network_close_socket(ctx->server_socket);
        ctx->server_socket = -1;
    }
    
    debug_print("SOAP Context pulito");
}
