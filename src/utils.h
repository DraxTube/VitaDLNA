#ifndef UTILS_H
#define UTILS_H

#include <psp2/types.h>

// Stampa debug su schermo
void debug_print(const char *format, ...);

// Pulisci stringa da caratteri speciali
void sanitize_string(char *str, int max_len);

// Estrai URL da richiesta SOAP
int extract_url_from_soap(const char *soap_request, char *url_buffer, int buffer_size);

// Estrai azione da richiesta SOAP
int extract_action_from_soap(const char *soap_request, char *action_buffer, int buffer_size);

// Genera risposta SOAP
int generate_soap_response(const char *action, char *response_buffer, int buffer_size);

// Genera descrizione dispositivo XML
int generate_device_description(char *xml_buffer, int buffer_size, const char *ip_address);

// Converte millisecondi in formato time (HH:MM:SS)
int format_time_ms(int ms, char *time_buffer, int buffer_size);

// Log su file (se disponibile)
void log_message(const char *level, const char *message);

#endif
