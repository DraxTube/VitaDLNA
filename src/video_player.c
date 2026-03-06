#include <psp2/kernel/processmgr.h>
#include <psp2/sysmodule.h>
#include <psp2/avplayer.h>
#include <stdio.h>
#include <string.h>

#include "video_player.h"
#include "utils.h"

int video_player_init(VideoPlayerContext *ctx) {
    if (!ctx) return -1;
    
    memset(ctx, 0, sizeof(VideoPlayerContext));
    
    // Carica modulo AvPlayer
    int ret = sceSysmoduleLoadModule(SCE_SYSMODULE_AVPPLAYER);
    if (ret < 0) {
        debug_print("Errore caricamento modulo AvPlayer: %d", ret);
        // Continua comunque, potremmo usare decoder alternativi
    }
    
    ctx->is_initialized = 1;
    ctx->is_playing = 0;
    ctx->current_url[0] = '\0';
    ctx->position_ms = 0;
    ctx->duration_ms = 0;
    
    debug_print("Video Player inizializzato");
    return 0;
}

int video_player_play(VideoPlayerContext *ctx, const char *url) {
    if (!ctx || !url || !ctx->is_initialized) return -1;
    
    strncpy(ctx->current_url, url, sizeof(ctx->current_url) - 1);
    ctx->current_url[sizeof(ctx->current_url) - 1] = '\0';
    
    debug_print("Riproduzione URL: %s", ctx->current_url);
    
    // NOTA: SceAvPlayer richiede configurazione complessa per stream di rete
    // Questa è un'implementazione semplificata
    
    // In produzione, qui dovresti:
    // 1. Scaricare/streammare il video
    // 2. Decodificare con SceAvPlayer
    // 3. Renderizzare su schermo
    
    ctx->is_playing = 1;
    ctx->position_ms = 0;
    
    debug_print("Riproduzione avviata");
    return 0;
}

int video_player_stop(VideoPlayerContext *ctx) {
    if (!ctx || !ctx->is_initialized) return -1;
    
    ctx->is_playing = 0;
    ctx->position_ms = 0;
    
    debug_print("Riproduzione fermata");
    return 0;
}

int video_player_pause(VideoPlayerContext *ctx) {
    if (!ctx || !ctx->is_initialized || !ctx->is_playing) return -1;
    
    // Implementa pausa
    debug_print("Riproduzione in pausa");
    return 0;
}

int video_player_resume(VideoPlayerContext *ctx) {
    if (!ctx || !ctx->is_initialized) return -1;
    
    ctx->is_playing = 1;
    debug_print("Riproduzione ripresa");
    return 0;
}

int video_player_get_position(VideoPlayerContext *ctx) {
    if (!ctx || !ctx->is_initialized) return 0;
    
    // Restituisci posizione corrente (in ms)
    // In produzione, ottieni da SceAvPlayer
    return ctx->position_ms;
}

int video_player_get_duration(VideoPlayerContext *ctx) {
    if (!ctx || !ctx->is_initialized) return 0;
    
    // Restituisci durata totale (in ms)
    // In produzione, ottieni da SceAvPlayer
    return ctx->duration_ms;
}

int video_player_is_playing(VideoPlayerContext *ctx) {
    if (!ctx) return 0;
    return ctx->is_playing;
}

void video_player_cleanup(VideoPlayerContext *ctx) {
    if (!ctx || !ctx->is_initialized) return;
    
    video_player_stop(ctx);
    
    // Scarica modulo AvPlayer
    sceSysmoduleUnloadModule(SCE_SYSMODULE_AVPPLAYER);
    
    ctx->is_initialized = 0;
    debug_print("Video Player pulito");
}
