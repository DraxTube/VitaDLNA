#ifndef VIDEO_PLAYER_H
#define VIDEO_PLAYER_H

#include <psp2/types.h>

typedef struct {
    int is_initialized;
    int is_playing;
    char current_url[1024];
    int position_ms;
    int duration_ms;
} VideoPlayerContext;

// Inizializza player video
int video_player_init(VideoPlayerContext *ctx);

// Avvia riproduzione
int video_player_play(VideoPlayerContext *ctx, const char *url);

// Ferma riproduzione
int video_player_stop(VideoPlayerContext *ctx);

// Pausa riproduzione
int video_player_pause(VideoPlayerContext *ctx);

// Riprendi riproduzione
int video_player_resume(VideoPlayerContext *ctx);

// Ottieni posizione corrente (ms)
int video_player_get_position(VideoPlayerContext *ctx);

// Ottieni durata totale (ms)
int video_player_get_duration(VideoPlayerContext *ctx);

// Controlla se sta riproducendo
int video_player_is_playing(VideoPlayerContext *ctx);

// Termina player
void video_player_cleanup(VideoPlayerContext *ctx);

#endif
