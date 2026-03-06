#include <psp2/kernel/processmgr.h>
#include <psp2/net/net.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "http_server.h"
#include "network.h"
#include "utils.h"

static const char *DEVICE_DESCRIPTION_TEMPLATE = 
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
    "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">\r\n"
    "  <specVersion>\r\n"
    "    <major>1</major>\r\n"
    "    <minor>0</minor>\r\n"
    "  </specVersion>\r\n"
    "  <device>\r\n"
    "    <deviceType>urn:schemas-upnp-org:device:MediaRenderer:1</deviceType>\r\n"
    "    <friendlyName>PS Vita DLNA Receiver</friendlyName>\r\n"
    "    <manufacturer>Sony</manufacturer>\r\n"
    "    <manufacturerURL>http://www.sony.com</manufacturerURL>\r\n"
    "    <modelDescription>PS Vita Media Renderer</modelDescription>\r\n"
    "    <modelName>PS Vita</modelName>\r\n"
    "    <modelNumber>1000</modelNumber>\r\n"
    "    <modelURL>http://www.sony.com/psvita</modelURL>\r\n"
    "    <serialNumber>VitaDLNA001</serialNumber>\r\n"
    "    <UDN>uuid:VitaDLNA-001</UDN>\r\n"
    "    <serviceList>\r\n"
    "      <service>\r\n"
    "        <serviceType>urn:schemas-upnp-org:service:AVTransport:1</serviceType>\r\n"
    "        <serviceId>urn:upnp-org:serviceId:AVTransport</serviceId>\r\n"
    "        <controlURL>http://%s:%d/control</controlURL>\r\n"
    "        <eventSubURL>http://%s:%d/event</eventSubURL>\r\n"
    "        <SCPDURL>http://%s:%d/AVTransport.xml</SCPDURL>\r\n"
    "      </service>\r\n"
    "      <service>\r\n"
    "        <serviceType>urn:schemas-upnp-org:service:RenderingControl:1</serviceType>\r\n"
    "        <serviceId>urn:upnp-org:serviceId:RenderingControl</serviceId>\r\n"
    "        <controlURL>http://%s:%d/control</controlURL>\r\n"
    "        <eventSubURL>http://%s:%d/event</eventSubURL>\r\n"
    "        <SCPDURL>http://%s:%d/RenderingControl.xml</SCPDURL>\r\n"
    "      </service>\r\n"
    "    </serviceList>\r\n"
    "    <presentationURL>http://%s:%d/</presentationURL>\r\n"
    "  </device>\r\n"
    "</root>\r\n";

int http_init(HTTPContext *ctx, DLNAContext *dlna_ctx, const char *ip_address) {
    if (!ctx || !dlna_ctx || !ip_address) return -1;
    
    memset(ctx, 0, sizeof(HTTPContext));
    ctx->dlna_ctx = dlna_ctx;
    
    // Crea socket server
    ctx->server_socket = network_create_tcp_socket();
    if (ctx->server_socket < 0) {
        debug_print("Errore creazione socket HTTP");
        return -1;
    }
    
    // Configura indirizzo server
    memset(&ctx->server_addr, 0, sizeof(ctx->server_addr));
    ctx->server_addr.sin_family = SCE_NET_AF_INET;
    ctx->server_addr.sin_port = sceNetHtons(HTTP_PORT);
    // sceNetInetPton(SCE_NET_AF_INET, ip_address, &ctx->server_addr.sin_addr);
    
    ctx->is_running = 0;
    
    debug_print("HTTP Context inizializzato");
    return 0;
}

int http_start(HTTPContext *ctx) {
    if (!ctx) return -1;
    
    // Bind socket
    int ret = network_bind_socket(ctx->server_socket, "0.0.0.0", HTTP_PORT);
    if (ret < 0) {
        debug_print("Errore bind socket HTTP: %d", ret);
        return -1;
    }
    
    // Listen
    ret = network_listen_socket(ctx->server_socket, 5);
    if (ret < 0) {
        debug_print("Errore listen socket HTTP: %d", ret);
        return -1;
    }
    
    ctx->is_running = 1;
    debug_print("HTTP server in ascolto su porta %d", HTTP_PORT);
    
    return 0;
}

int http_process_request(HTTPContext *ctx, int client_socket) {
    if (!ctx || client_socket < 0) return -1;
    
    char buffer[MAX_BUFFER];
    memset(buffer, 0, sizeof(buffer));
    
    // Ricevi richiesta HTTP
    int received = network_recv_data(client_socket, buffer, sizeof(buffer) - 1);
    if (received <= 0) {
        debug_print("Errore ricezione richiesta HTTP");
        return -1;
    }
    
    debug_print("Ricevuta richiesta HTTP: %.100s", buffer);
    
    // Controlla tipo di richiesta
    if (strstr(buffer, "GET /description.xml")) {
        http_send_device_description(ctx, client_socket);
    } else if (strstr(buffer, "GET /")) {
        http_send_device_description(ctx, client_socket);
    } else {
        http_send_404(ctx, client_socket);
    }
    
    return 0;
}

int http_send_device_description(HTTPContext *ctx, int client_socket) {
    if (!ctx || client_socket < 0) return -1;
    
    char xml_description[MAX_BUFFER * 2];
    char ip_str[16] = "192.168.1.100"; // Da ottenere dinamicamente
    
    snprintf(xml_description, sizeof(xml_description), DEVICE_DESCRIPTION_TEMPLATE,
             ip_str, SOAP_PORT, ip_str, SOAP_PORT, ip_str, SOAP_PORT,
             ip_str, SOAP_PORT, ip_str, SOAP_PORT, ip_str, SOAP_PORT,
             ip_str, HTTP_PORT);
    
    char response[MAX_BUFFER * 2];
    snprintf(response, sizeof(response),
             "HTTP/1.1 200 OK\r\n"
             "CONTENT-TYPE: text/xml; charset=\"utf-8\"\r\n"
             "CONTENT-LENGTH: %d\r\n"
             "SERVER: PS Vita DLNA/1.0\r\n"
             "CONNECTION: close\r\n"
             "EXT: \r\n"
             "\r\n"
             "%s",
             (int)strlen(xml_description), xml_description);
    
    network_send_data(client_socket, response, strlen(response));
    debug_print("Descrizione dispositivo inviata");
    
    return 0;
}

int http_send_404(HTTPContext *ctx, int client_socket) {
    if (!ctx || client_socket < 0) return -1;
    
    const char *response = 
        "HTTP/1.1 404 Not Found\r\n"
        "CONTENT-TYPE: text/html\r\n"
        "CONTENT-LENGTH: 0\r\n"
        "SERVER: PS Vita DLNA/1.0\r\n"
        "CONNECTION: close\r\n"
        "\r\n";
    
    network_send_data(client_socket, response, strlen(response));
    debug_print("Risposta 404 inviata");
    
    return 0;
}

void http_stop(HTTPContext *ctx) {
    if (!ctx || !ctx->is_running) return;
    
    ctx->is_running = 0;
    debug_print("HTTP server arrestato");
}

void http_cleanup(HTTPContext *ctx) {
    if (!ctx) return;
    
    if (ctx->server_socket >= 0) {
        network_close_socket(ctx->server_socket);
        ctx->server_socket = -1;
    }
    
    debug_print("HTTP Context pulito");
}
