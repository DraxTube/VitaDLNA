#include <psp2/kernel/processmgr.h>
#include <psp2/ctrl.h>
#include <psp2/display.h>
#include <psp2/sysmodule.h>
#include <psp2/appmgr.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "dlna.h"
#include "network.h"
#include "utils.h"
#include "video_player.h"

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 544

static DLNAContext g_dlna_ctx;
static VideoPlayerContext g_player_ctx;
static int g_running = 1;

void draw_text(int x, int y, const char *text, unsigned int color) {
    // Implementazione base per debug su schermo
    // In produzione usare librerie grafiche come vitas2d
    (void)x; (void)y; (void)text; (void)color;
}

void draw_status_screen(DLNAContext *dlna, VideoPlayerContext *player) {
    // Pulisci schermo (implementazione semplificata)
    
    // Mostra stato
    debug_print("VitaDLNA Receiver v1.0");
    debug_print("IP: %s", dlna->ip_address);
    debug_print("Status: %s", dlna->is_running ? "RUNNING" : "STOPPED");
    debug_print("Playing: %s", player->is_playing ? "YES" : "NO");
    
    if (player->is_playing && player->current_url[0] != '\0') {
        debug_print("URL: %.50s...", player->current_url);
        debug_print("Position: %d ms", player->position_ms);
    }
    
    debug_print("Premi X per uscire");
}

int handle_input() {
    SceCtrlData pad;
    sceCtrlPeekBufferPositive(0, &pad, 1);
    
    if (pad.buttons & SCE_CTRL_CROSS) {
        return 0; // Esci
    }
    
    return 1; // Continua
}

int main(int argc, char *argv[]) {
    int ret;
    
    // Inizializza moduli di sistema
    sceSysmoduleLoadModule(SCE_SYSMODULE_CTRL);
    sceSysmoduleLoadModule(SCE_SYSMODULE_DISPLAY);
    
    // Inizializza rete
    ret = network_init();
    if (ret < 0) {
        debug_print("Errore inizializzazione rete: %d", ret);
        sceKernelDelayThread(2000000);
        sceKernelExitProcess(-1);
    }
    
    // Ottieni indirizzo IP
    ret = network_get_ip_address(g_dlna_ctx.ip_address, sizeof(g_dlna_ctx.ip_address));
    if (ret < 0) {
        debug_print("Errore ottenimento IP: %d", ret);
        strcpy(g_dlna_ctx.ip_address, "0.0.0.0");
    }
    
    debug_print("IP Address: %s", g_dlna_ctx.ip_address);
    
    // Inizializza player video
    ret = video_player_init(&g_player_ctx);
    if (ret < 0) {
        debug_print("Errore inizializzazione player: %d", ret);
    }
    
    // Inizializza server DLNA
    ret = dlna_init(&g_dlna_ctx);
    if (ret < 0) {
        debug_print("Errore inizializzazione DLNA: %d", ret);
        sceKernelDelayThread(2000000);
        sceKernelExitProcess(-1);
    }
    
    // Collega player al contesto DLNA
    g_dlna_ctx.is_playing = 0;
    
    // Avvia server DLNA
    ret = dlna_start(&g_dlna_ctx);
    if (ret < 0) {
        debug_print("Errore avvio DLNA: %d", ret);
        sceKernelDelayThread(2000000);
        sceKernelExitProcess(-1);
    }
    
    debug_print("Server DLNA avviato!");
    debug_print("Apri Web Video Caster e cerca dispositivi");
    
    // Loop principale
    while (g_running) {
        // Aggiorna stato player
        if (g_player_ctx.is_playing) {
            g_player_ctx.position_ms = video_player_get_position(&g_player_ctx);
            g_dlna_ctx.is_playing = 1;
        } else {
            g_dlna_ctx.is_playing = 0;
        }
        
        // Disegna stato su schermo
        draw_status_screen(&g_dlna_ctx, &g_player_ctx);
        
        // Controlla input
        if (!handle_input()) {
            g_running = 0;
        }
        
        // Piccolo delay per non consumare troppa CPU
        sceKernelDelayThread(50000); // 50ms
    }
    
    // Cleanup
    debug_print("Arresto in corso...");
    
    dlna_stop(&g_dlna_ctx);
    video_player_cleanup(&g_player_ctx);
    dlna_cleanup(&g_dlna_ctx);
    network_cleanup();
    
    sceSysmoduleUnloadModule(SCE_SYSMODULE_DISPLAY);
    sceSysmoduleUnloadModule(SCE_SYSMODULE_CTRL);
    
    debug_print("Arrivederci!");
    sceKernelDelayThread(2000000);
    
    sceKernelExitProcess(0);
    return 0;
}
