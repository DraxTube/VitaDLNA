#ifndef NETWORK_H
#define NETWORK_H

#include <psp2/types.h>

// Inizializza sistema di rete
int network_init(void);

// Ottieni indirizzo IP locale
int network_get_ip_address(char *ip_buffer, int buffer_size);

// Crea socket TCP
int network_create_tcp_socket(void);

// Crea socket UDP
int network_create_udp_socket(void);

// Bind socket a porta
int network_bind_socket(int socket_fd, const char *ip, int port);

// Ascolta connessioni
int network_listen_socket(int socket_fd, int backlog);

// Accetta connessione
int network_accept_socket(int server_socket, struct sockaddr_in *client_addr);

// Invia dati
int network_send_data(int socket_fd, const char *data, int data_size);

// Ricevi dati
int network_recv_data(int socket_fd, char *buffer, int buffer_size);

// Chiudi socket
int network_close_socket(int socket_fd);

// Termina sistema di rete
void network_cleanup(void);

#endif
