#include "audio.h"
#include <stdio.h>
#include <string.h>

#ifdef HAVE_SDL2_MIXER
#include <SDL2/SDL_mixer.h>
static Mix_Music *musica_atual = NULL;
static int audio_inicializado = 0;
#else
static int audio_inicializado = 0;
#endif

// Flag para indicar se é a fase média (para volume aumentado)
static int e_fase_media = 0;

// Flag para indicar se a música está ligada (0 = desligada, 1 = ligada)
static int musica_ligada = 0;

int inicializar_audio(void) {
#ifdef HAVE_SDL2_MIXER
    // Inicializar SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        fprintf(stderr, "Aviso: Não foi possível inicializar áudio: %s\n", Mix_GetError());
        return 0;
    }
    
    // Permitir MP3
    int flags = MIX_INIT_MP3;
    if ((Mix_Init(flags) & flags) != flags) {
        fprintf(stderr, "Aviso: Suporte MP3 não disponível: %s\n", Mix_GetError());
        // Continuar mesmo sem MP3, pode ser que funcione de qualquer forma
    }
    
    audio_inicializado = 1;
    return 1;
#else
    fprintf(stderr, "Aviso: SDL2_mixer não está disponível. Áudio desabilitado.\n");
    fprintf(stderr, "Para habilitar áudio, instale: sudo apt-get install libsdl2-mixer-dev\n");
    audio_inicializado = 0;
    return 0;
#endif
}

int audio_disponivel(void) {
    return audio_inicializado;
}

void definir_dificuldade_musica(int e_fase_media_flag) {
    e_fase_media = e_fase_media_flag;
}

void definir_musica_ligada(int ligada) {
    musica_ligada = ligada;
    // Se desligar a música, definir volume como 0 imediatamente
    if (!ligada && audio_inicializado) {
#ifdef HAVE_SDL2_MIXER
        Mix_VolumeMusic(0);
#endif
    }
}

int tocar_musica(const char* arquivo) {
#ifdef HAVE_SDL2_MIXER
    // Se já tem música tocando, parar primeiro
    if (musica_atual) {
        Mix_HaltMusic();
        Mix_FreeMusic(musica_atual);
        musica_atual = NULL;
    }
    
    musica_atual = Mix_LoadMUS(arquivo);
    if (!musica_atual) {
        fprintf(stderr, "Aviso: Não foi possível carregar música %s: %s\n", arquivo, Mix_GetError());
        return 0;
    }
    
    // Ajustar volume baseado no estado da música e dificuldade
    int volume = 0; // Volume padrão (silencioso se música desligada)
    if (musica_ligada) {
        volume = 64; // Volume padrão (50%)
        if (e_fase_media) {
            volume = 80; // 25% mais alto que 64 para fase média
        }
    }
    Mix_VolumeMusic(volume);
    
    // Tocar música em loop (-1 = loop infinito)
    if (Mix_PlayMusic(musica_atual, -1) < 0) {
        fprintf(stderr, "Aviso: Não foi possível tocar música: %s\n", Mix_GetError());
        Mix_FreeMusic(musica_atual);
        musica_atual = NULL;
        return 0;
    }
    
    return 1;
#else
    (void)arquivo; // Evitar warning de parâmetro não usado
    return 0;
#endif
}

int tocar_musica_temporaria(const char* arquivo) {
#ifdef HAVE_SDL2_MIXER
    // Se já tem música tocando, parar primeiro
    if (musica_atual) {
        Mix_HaltMusic();
        Mix_FreeMusic(musica_atual);
        musica_atual = NULL;
    }
    
    musica_atual = Mix_LoadMUS(arquivo);
    if (!musica_atual) {
        fprintf(stderr, "Aviso: Não foi possível carregar música %s: %s\n", arquivo, Mix_GetError());
        return 0;
    }
    
    // Ajustar volume baseado no estado da música e dificuldade
    int volume = 0; // Volume padrão (silencioso se música desligada)
    if (musica_ligada) {
        volume = 64; // Volume padrão (50%)
        if (e_fase_media) {
            volume = 80; // 25% mais alto que 64 para fase média
        }
    }
    Mix_VolumeMusic(volume);
    
    // Tocar música uma vez (0 = sem loop)
    if (Mix_PlayMusic(musica_atual, 0) < 0) {
        fprintf(stderr, "Aviso: Não foi possível tocar música: %s\n", Mix_GetError());
        Mix_FreeMusic(musica_atual);
        musica_atual = NULL;
        return 0;
    }
    
    return 1;
#else
    (void)arquivo; // Evitar warning de parâmetro não usado
    return 0;
#endif
}

void parar_musica(void) {
#ifdef HAVE_SDL2_MIXER
    Mix_HaltMusic();
    if (musica_atual) {
        Mix_FreeMusic(musica_atual);
        musica_atual = NULL;
    }
#endif
}

int musica_tocando(void) {
#ifdef HAVE_SDL2_MIXER
    return Mix_PlayingMusic() == 1;
#else
    return 0;
#endif
}

void finalizar_audio(void) {
#ifdef HAVE_SDL2_MIXER
    parar_musica();
    Mix_CloseAudio();
    Mix_Quit();
#endif
}

