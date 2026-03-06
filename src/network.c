#include <psp2/kernel/processmgr.h>
#include <psp2/sysmodule.h>
#include <psp2/net/net.h>
#include <psp2/net/netctl.h>
#include <stdio.h>
#include <string.h>

#include "network.h"
#include "utils.h"

static int g_network_initialized = 0;

int network_init(void) {
    if (g_network_initialized) return 0;
    
    int ret;
    
    // Carica modulo Net
    ret = sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
    if (ret < 0) {
        debug_print("Errore caricamento modulo Net: %d", ret);
        return -1;
    }
    
    // Inizializza Net
    ret = sceNetInit(0x10000, 0x40, 0, 0, 0);
    if (ret < 0) {
        debug_print("Errore inizializzazione Net: %d", ret);
        return -1;
    }
    
    // Inizializza NetCtl
    ret = sceNetCtlInit();
    if (ret < 0) {
        debug_print("Errore inizializzazione NetCtl: %d", ret);
        sceNetTerm();
        return -1;
    }
    
    g_network_initialized = 1;
    debug_print("Sistema di rete inizializzato");
    return 0;
}

int network_get_ip_address(char *ip_buffer, int buffer_size) {
    if (!ip_buffer || buffer_size < 16) return -1;
    
    SceNetCtlInfo info;
    int ret;
    
    // Ottieni informazioni IP
    ret = sceNetCtlInetGetInfo(SCE_NETCTL_INFO_GET_IP_ADDRESS, &info);
    if (ret < 0) {
        debug_print("Errore ottenimento IP: %d", ret);
        strncpy(ip_buffer, "0.0.0.0", buffer_size - 1);
        ip_buffer[buffer_size - 1] = '\0';
        return -1;
    }
    
    strncpy(ip_buffer, info.ip_address, buffer_size - 1);
    ip_buffer[buffer_size - 1] = '\0';
    
    debug_print("Indirizzo IP: %s", ip_buffer);
    return 0;
}

int network_create_tcp_socket(void) {
    int sock = sceNetSocket("VitaTCP", SCE_NET_AF_INET, SCE_NET_SOCK_STREAM, 0);
    if (sock < 0) {
        debug_print("Errore creazione socket TCP: %d", sock);
        return -1;
    }
    
    debug_print("Socket TCP creato: %d", sock);
    return sock;
}

int network_create_udp_socket(void) {
    int sock = sceNetSocket("VitaUDP", SCE_NET_AF_INET, SCE_NET_SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_print("Errore creazione socket UDP: %d", sock);
        return -1;
    }
    
    debug_print("Socket UDP creato: %d", sock);
    return sock;
}

int network_bind_socket(int socket_fd, const char *ip, int port) {
    if (socket_fd < 0 || !ip) return -1;
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = SCE_NET_AF_INET;
    addr.sin_port = sceNetHtons(port);
    // sceNetInetPton(SCE_NET_AF_INET, ip, &addr.sin_addr);
    addr.sin_addr.s_addr = sceNetHtonl(0); // Bind su tutte le interfacce
    
    int ret = sceNetBind(socket_fd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        debug_print("Errore bind socket %d: %d", socket_fd, ret);
        return -1;
    }
    
    debug_print("Socket %d bindato su %s:%d", socket_fd, ip, port);
    return 0;
}

int network_listen_socket(int server_socket, int backlog) {
    if (server_socket < 0) return -1;
    
    int ret = sceNetListen(server_socket, backlog);
    if (ret < 0) {
        debug_print("Errore listen socket %d: %d", server_socket, ret);
        return -1;
    }
    
    debug_print("Socket %d in ascolto (backlog: %d)", server_socket, backlog);
    return 0;
}

int network_accept_socket(int server_socket, struct sockaddr_in *client_addr) {
    if (server_socket < 0 || !client_addr) return -1;
    
    SceUInt addr_size = sizeof(struct sockaddr_in);
    int client_socket = sceNetAccept(server_socket, (struct sockaddr *)client_addr, &addr_size);
    if (client_socket < 0) {
        debug_print("Errore accept socket: %d", client_socket);
        return -1;
    }
    
    debug_print("Connessione accettata: socket %d", client_socket);
    return client_socket;
}

int network_send_data(int socket_fd, const char *data, int data_size) {
    if (socket_fd < 0 || !data || data_size <= 0) return -1;
    
    int sent = sceNetSend(socket_fd, data, data_size, 0);
    if (sent < 0) {
        debug_print("Errore invio dati su socket %d: %d", socket_fd, sent);
        return -1;
    }
    
    debug_print("Dati inviati: %d byte", sent);
    return sent;
}

int network_recv_data(int socket_fd, char *buffer, int buffer_size) {
    if (socket_fd < 0 || !buffer || buffer_size <= 0) return -1;
    
    int received = sceNetRecv(socket_fd, buffer, buffer_size, 0);
    if (received < 0) {
        debug_print("Errore ricezione dati su socket %d: %d", socket_fd, received);
        return -1;
    }
    
    debug_print("Dati ricevuti: %d byte", received);
    return received;
}

int network_close_socket(int socket_fd) {
    if (socket_fd < 0) return -1;
    
    int ret = sceNetSocketClose(socket_fd);
    if (ret < 0) {
        debug_print("Errore chiusura socket %d: %d", socket_fd, ret);
        return -1;
    }
    
    debug_print("Socket %d chiuso", socket_fd);
    return 0;
}

void network_cleanup(void) {
    if (!g_network_initialized) return;
    
    sceNetCtlTerm();
    sceNetTerm();
    sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);
    
    g_network_initialized = 0;
    debug_print("Sistema di rete terminato");
}
