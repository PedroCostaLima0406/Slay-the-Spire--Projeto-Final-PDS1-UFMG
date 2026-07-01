#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <allegro5/allegro5.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include "combat.h"

#define CARD_TYPE_COUNT 8

typedef struct {
    ALLEGRO_BITMAP* card_images[CARD_TYPE_COUNT];
} CardAssets;

typedef struct Renderer {
  ALLEGRO_DISPLAY* display;
  ALLEGRO_BITMAP* display_buffer;

  ALLEGRO_FONT* font;

  Combat* combat;

  ALLEGRO_BITMAP* background;
  CardAssets cards;
  ALLEGRO_BITMAP* player_image;

  ALLEGRO_BITMAP* enemy_weak;
  ALLEGRO_BITMAP* enemy_strong;
  ALLEGRO_BITMAP* boss;
  
  ALLEGRO_BITMAP* deck_image;
  ALLEGRO_BITMAP* discard_image;
  
  ALLEGRO_BITMAP* icon_attack;
  ALLEGRO_BITMAP* icon_block;
  
  ALLEGRO_AUDIO_STREAM* music_normal;
  ALLEGRO_AUDIO_STREAM* music_boss;
  ALLEGRO_AUDIO_STREAM* music_gameover;

  ALLEGRO_AUDIO_STREAM* current_music;

  float player_draw_x;
  float player_draw_y;
  float player_draw_w;
  float player_draw_h;
} Renderer;

void FillRenderer(Renderer* renderer);

void Render(Renderer* renderer);

void ClearRenderer(Renderer* renderer);

void PlayMusic(Renderer* r, ALLEGRO_AUDIO_STREAM* stream, float volume);

#endif
