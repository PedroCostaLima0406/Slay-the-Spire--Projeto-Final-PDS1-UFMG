#include "renderer.h"

#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "constants.h"
#include "utils.h"

void DrawScaledText(ALLEGRO_FONT* font, ALLEGRO_COLOR color, float x, float y,
                    float xscale, float yscale, int alignment,
                    const char* text) {
  ALLEGRO_TRANSFORM transform;
  al_identity_transform(&transform);
  al_scale_transform(&transform, xscale, yscale);
  al_use_transform(&transform);

  al_draw_text(font, color, x, y, alignment, text);
  
  al_identity_transform(&transform);
  al_use_transform(&transform);
}

void DrawCenteredScaledText(ALLEGRO_FONT* font, ALLEGRO_COLOR color, float x,
                            float y, float xscale, float yscale, char* text) {
  DrawScaledText(font, color, x, y, xscale, yscale, ALLEGRO_ALIGN_CENTRE, text);
}

// desenha texto centralizado nas telas de menu e combate
void DrawLargeCenteredText(ALLEGRO_FONT* font, ALLEGRO_COLOR color, float center_x, float center_y, float scale, const char* text) {
    if (!font || !text) return;
    
    ALLEGRO_TRANSFORM prev, transform;
    al_copy_transform(&prev, al_get_current_transform());
    al_identity_transform(&transform);
    al_scale_transform(&transform, scale, scale);
    al_use_transform(&transform);
    
    al_draw_text(font, color, center_x / scale, center_y / scale, ALLEGRO_ALIGN_CENTER, text);
    
    al_use_transform(&prev);
}

// forward declaration para evitar erro de declaração implícita 
void RenderTurnNumber(struct Renderer* r, int turn_number, int total_turns);
// forward declarations para funções definidar mais abaixo no código (deu preguiça de colocar no .h)
void RenderCard(struct Renderer* r, Card* card, int x_left, int y_top, int selected);
void RenderPlayer(struct Renderer* r);

void FillRenderer(Renderer* renderer) {
    // display
    al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, 1, ALLEGRO_SUGGEST);
    al_set_new_display_option(ALLEGRO_SAMPLES, 8, ALLEGRO_SUGGEST);
    al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
    
    renderer->display = al_create_display(DISPLAY_WIDTH, DISPLAY_HEIGHT);
    must_init(renderer->display, "display");

    if (!renderer->display) exit(1);

    // addons
    must_init(al_init_image_addon(), "image addon");
    must_init(al_init_primitives_addon(), "primitives addon");
    must_init(al_init_font_addon(), "font addon");
    must_init(al_install_keyboard(), "keyboard");
    
    // audio
    bool audio_available = al_install_audio();
    if (audio_available) {
        al_init_acodec_addon();
        al_reserve_samples(16);
        
        ALLEGRO_MIXER* mixer = al_get_default_mixer();
        if (mixer) {
            al_set_mixer_quality(mixer, ALLEGRO_MIXER_QUALITY_LINEAR);
        }
    }

    al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR);

    // assets
    renderer->background = al_load_bitmap("assets/background.jpg");
    must_init(renderer->background, "background");

    renderer->cards.card_images[CARD_ATTACK] = al_load_bitmap("assets/carta_ataque.png"); must_init(renderer->cards.card_images[CARD_ATTACK], "card attack");
    renderer->cards.card_images[CARD_DEFENSE] = al_load_bitmap("assets/carta_escudo.png"); must_init(renderer->cards.card_images[CARD_DEFENSE], "card defense");
    renderer->cards.card_images[CARD_SPECIAL] = al_load_bitmap("assets/carta_especial.png"); must_init(renderer->cards.card_images[CARD_SPECIAL], "card special");
    renderer->cards.card_images[CARD_HEAL] = al_load_bitmap("assets/carta_cura.png"); must_init(renderer->cards.card_images[CARD_HEAL], "card heal");
    renderer->cards.card_images[CARD_BUFF_DAMAGE] = al_load_bitmap("assets/carta_buff_dano.png"); must_init(renderer->cards.card_images[CARD_BUFF_DAMAGE], "buff damage card");
    renderer->cards.card_images[CARD_DEBUFF_DEFENSE] = al_load_bitmap("assets/carta_debuff_escudo.png"); must_init(renderer->cards.card_images[CARD_DEBUFF_DEFENSE], "card debuff");
    renderer->cards.card_images[CARD_ATTACK_HEAL] = al_load_bitmap("assets/carta_ataque_e_cura.png"); must_init(renderer->cards.card_images[CARD_ATTACK_HEAL], "card attack+heal");
    renderer->cards.card_images[CARD_JOKER] = al_load_bitmap("assets/carta_joker.png"); must_init(renderer->cards.card_images[CARD_JOKER], "card joker");

    renderer->deck_image = al_load_bitmap("assets/deck.png"); must_init(renderer->deck_image, "deck image");
    renderer->discard_image = al_load_bitmap("assets/descarte.png"); must_init(renderer->discard_image, "discard image");

    renderer->player_image = al_load_bitmap("assets/player.png"); must_init(renderer->player_image, "player image");
    renderer->enemy_weak = al_load_bitmap("assets/inimigo_fraco.png"); must_init(renderer->enemy_weak, "enemy weak");
    renderer->enemy_strong = al_load_bitmap("assets/inimigo_forte.png"); must_init(renderer->enemy_strong, "enemy strong");
    renderer->boss = al_load_bitmap("assets/boss.png"); must_init(renderer->boss, "boss");

    renderer->icon_attack = al_load_bitmap("assets/icon_attack.png"); must_init(renderer->icon_attack, "icon attack");
    renderer->icon_block = al_load_bitmap("assets/icon_block.png"); must_init(renderer->icon_block, "icon block");

    // música
    renderer->music_normal = NULL;
    renderer->music_boss = NULL;
    renderer->music_gameover = NULL;
    renderer->current_music = NULL;
    
    if (al_is_audio_installed()) {
        renderer->music_normal = al_load_audio_stream("assets/music_normal.wav", 16, 8192);
        renderer->music_boss = al_load_audio_stream("assets/music_boss.wav", 16, 8192);
        renderer->music_gameover = al_load_audio_stream("assets/music_gameover.wav", 16, 8192);
        
        if (renderer->music_normal) {
            al_attach_audio_stream_to_mixer(renderer->music_normal, al_get_default_mixer());
            al_set_audio_stream_playing(renderer->music_normal, false);
        }
        if (renderer->music_boss) {
            al_attach_audio_stream_to_mixer(renderer->music_boss, al_get_default_mixer());
            al_set_audio_stream_playing(renderer->music_boss, false);
        }
        if (renderer->music_gameover) {
            al_attach_audio_stream_to_mixer(renderer->music_gameover, al_get_default_mixer());
            al_set_audio_stream_playing(renderer->music_gameover, false);
        }
    }

    renderer->display_buffer = al_create_bitmap((int) DISPLAY_BUFFER_WIDTH, (int) DISPLAY_BUFFER_HEIGHT);
    must_init(renderer->display_buffer, "display buffer");

    renderer->font = al_create_builtin_font();
    must_init(renderer->font, "font");
}

void RenderBackground(Renderer* renderer) {
  al_draw_scaled_bitmap(
        renderer->background,
        0, 0, 
        al_get_bitmap_width(renderer->background),
        al_get_bitmap_height(renderer->background),
        0, 0,
        DISPLAY_BUFFER_WIDTH,
        DISPLAY_BUFFER_HEIGHT,
        0
  );
}

void RenderDeck(Renderer* r, Deck* deck, int x_left, int y_top, int show_discard) {
    ALLEGRO_BITMAP* prev_bmp = al_get_target_bitmap();
    ALLEGRO_BITMAP* deck_bitmap = al_create_bitmap(DECK_WIDTH, DECK_HEIGHT);
    al_set_target_bitmap(deck_bitmap);
    
    al_draw_filled_rounded_rectangle(0, 0, DECK_WIDTH, DECK_HEIGHT, 10, 0, al_map_rgb(255, 255, 255));
    al_draw_rounded_rectangle(0, 0, DECK_WIDTH, DECK_HEIGHT, 10, 0, al_map_rgb(0, 0, 0), 2);
	
    al_set_target_bitmap(prev_bmp);

    al_draw_scaled_bitmap(deck_bitmap, 0, 0, DECK_WIDTH, DECK_HEIGHT, x_left, y_top, DECK_WIDTH, DECK_HEIGHT, 0);
    al_destroy_bitmap(deck_bitmap);

    char draw_count[8];
    sprintf(draw_count, "%d", deck->draw_pile.size);
    DrawScaledText(r->font, al_map_rgb(40, 0, 0), x_left + DECK_WIDTH + 10, y_top + DECK_HEIGHT/2,
                   2.0, 2.0, ALLEGRO_ALIGN_LEFT, draw_count);

    if (show_discard) {
        int discard_x = x_left + 2*DECK_WIDTH + 50;
        ALLEGRO_BITMAP* discard_bitmap = al_create_bitmap(DECK_WIDTH, DECK_HEIGHT);
        al_set_target_bitmap(discard_bitmap);
        al_draw_filled_rounded_rectangle(0, 0, DECK_WIDTH, DECK_HEIGHT, 10, 0, al_map_rgb(180, 180, 180));
        al_draw_rounded_rectangle(0, 0, DECK_WIDTH, DECK_HEIGHT, 10, 0, al_map_rgb(0, 0, 0), 2);
        al_set_target_bitmap(prev_bmp);
        al_draw_scaled_bitmap(discard_bitmap, 0, 0, DECK_WIDTH, DECK_HEIGHT, discard_x, y_top, DECK_WIDTH, DECK_HEIGHT, 0);
        al_destroy_bitmap(discard_bitmap);

        char discard_count[8];
        sprintf(discard_count, "%d", deck->discard_pile.size);
        DrawScaledText(r->font, al_map_rgb(40, 0, 0), discard_x + DECK_WIDTH + 10, y_top + DECK_HEIGHT/2,
                       2.0, 2.0, ALLEGRO_ALIGN_LEFT, discard_count);
    }
}


void RenderHealthBarFull(float x_begin, float x_end, float y_top,int hp, int max_hp, int block, ALLEGRO_FONT* font) {
    float bar_width = x_end - x_begin;
    float bar_height = HEALTH_BAR_HEIGHT;

    float hp_ratio = (float)hp / (float)max_hp;
    if (hp_ratio < 0) hp_ratio = 0;
    if (hp_ratio > 1) hp_ratio = 1;

    // fundo
    al_draw_filled_rectangle(
        x_begin, y_top, x_end, y_top + bar_height, al_map_rgb(60, 0, 0)
    );

    // barra
    al_draw_filled_rectangle(
        x_begin, y_top, x_begin + bar_width * hp_ratio,
        y_top + bar_height, al_map_rgb(200, 0, 0)
    );

    // borda
    al_draw_rectangle(
        x_begin, y_top, x_end, y_top + bar_height, al_map_rgb(0, 0, 0), 2
    );

    // texto da vida
    if (font) {
        ALLEGRO_TRANSFORM ident;
        al_identity_transform(&ident);
        al_use_transform(&ident);

        char hpbuf[64];
        sprintf(hpbuf, "%d / %d", hp, max_hp);
        float mid_x = (x_begin + x_end) * 0.5f;
        float mid_y = y_top + (bar_height / 2.0f) - (al_get_font_line_height(font) / 2.0f);
        al_draw_text(font, al_map_rgb(255,255,255), mid_x, mid_y, ALLEGRO_ALIGN_CENTER, hpbuf);

            if (block > 0) {
            char blkbuf[32];
            sprintf(blkbuf, "%d", block);
            const float b_xscale = 1.9f;
            const float b_yscale = 1.9f;
            float bx = (x_end + 12.0f) / b_xscale;
            float by = (y_top + (bar_height / 2.0f) - (al_get_font_line_height(font) / 2.0f)) / b_yscale;
            DrawScaledText(font, al_map_rgb(255,255,255), bx, by, b_xscale, b_yscale, ALLEGRO_ALIGN_LEFT, blkbuf);
        }
        al_identity_transform(&ident);
        al_use_transform(&ident);
    }
}

// renderiza uma carta
void RenderCard(struct Renderer* r, Card* card, int x_left, int y_top, int selected) {
    ALLEGRO_BITMAP* card_bmp = al_create_bitmap(CARD_WIDTH, CARD_HEIGHT);
    if (!card_bmp) return;

    ALLEGRO_BITMAP* prev = al_get_target_bitmap();
    if (!prev) {
        al_destroy_bitmap(card_bmp);
        return;
    }

    al_set_target_bitmap(card_bmp);
    al_clear_to_color(al_map_rgba(0,0,0,0));

    ALLEGRO_BITMAP* base = NULL;
    if (card && card->type >= 0 && card->type < CARD_TYPE_COUNT) base = r->cards.card_images[card->type];
    if (!base) base = r->cards.card_images[CARD_ATTACK];
    if (base) {
        al_draw_scaled_bitmap(
            base, 0, 0, al_get_bitmap_width(base), al_get_bitmap_height(base),
            0, 0, CARD_WIDTH, CARD_HEIGHT, 0
        );
    } else {
        al_draw_filled_rounded_rectangle(0, 0, CARD_WIDTH, CARD_HEIGHT, 8, 0, al_map_rgb(100,100,100));
    }

    if (card) {
        if (r && r->font) {
            const float text_xscale = 2.4f;
            const float text_yscale = 2.4f;

            float cost_px = 14.0f;
            float cost_py = 12.0f;
            char cost_str[16];
            sprintf(cost_str, "%d", card->cost);
            DrawScaledText(r->font, al_map_rgb(0,0,0), cost_px / text_xscale, cost_py / text_yscale, text_xscale, text_yscale, ALLEGRO_ALIGN_LEFT, cost_str);

            float effect_px = CARD_WIDTH - 14.0f;
            float effect_py = 12.0f;
            char effect_str[16];
            sprintf(effect_str, "%d", card->effect);
            DrawScaledText(r->font, al_map_rgb(30,140,200), effect_px / text_xscale, effect_py / text_yscale, text_xscale, text_yscale, ALLEGRO_ALIGN_RIGHT, effect_str);
        }
    }

    al_set_target_bitmap(prev);

    float hand_card_scale = 0.95f;
    float draw_w = CARD_WIDTH * hand_card_scale;
    float draw_h = CARD_HEIGHT * hand_card_scale;
    float draw_x = (float)x_left;
    float draw_y = (float)y_top;
    if (selected) draw_y -= 20.0f;

    al_draw_scaled_bitmap(card_bmp, 0, 0, CARD_WIDTH, CARD_HEIGHT, draw_x, draw_y, draw_w, draw_h, 0);
    al_destroy_bitmap(card_bmp);
}

// renderiza o sprite do jogador
void RenderPlayer(struct Renderer* r) {
    if (!r || !r->combat) return;
    Player* p = &r->combat->player;
    if (!r->player_image) return;

    ALLEGRO_BITMAP* bmp = r->player_image;
    int bw = al_get_bitmap_width(bmp);
    int bh = al_get_bitmap_height(bmp);

    float draw_w = r->player_draw_w;
    float draw_h = r->player_draw_h;
    float x = r->player_draw_x;
    float y = r->player_draw_y;

    // fallback
    if (draw_w <= 0 || draw_h <= 0) {
        float max_allowed_w = CARD_WIDTH * 0.9f;
        float scale = max_allowed_w / (float)bw;
        if (scale > 0.9f) scale = 0.9f;
        if (scale < 0.15f) scale = 0.15f;
        draw_w = bw * scale;
        draw_h = bh * scale;
        x = 40.0f;
        y = DISPLAY_BUFFER_HEIGHT * 0.75f - draw_h - 16.0f;
    }

    // adiciona animações de highlight em caso de dano/cura ou dano e cura
    if (p->highlight_frames > 0) {
        al_draw_tinted_scaled_bitmap(bmp, al_map_rgba(255, 0, 0, 128), 0, 0, bw, bh, x, y, draw_w, draw_h, 0);
    } else {
        al_draw_scaled_bitmap(bmp, 0, 0, bw, bh, x, y, draw_w, draw_h, 0);
    }

    RenderHealthBarFull(x, x + draw_w, y + draw_h + 8.0f, p->hp, p->max_hp, p->block, r->font);
}


void RenderPlayerHand(Renderer* r) {
    if (!r || !r->combat) return;
    if (!r->player_image) return;

    Deck* deck = &r->combat->player.deck;
    int count = deck->hand.size;

    // computa as dimensões do display das cartas
    float hand_card_scale = 0.95f;
    float card_spacing = CARD_WIDTH * hand_card_scale + 8.0f;
    float draw_w_card = CARD_WIDTH * hand_card_scale;
    float draw_h_card = CARD_HEIGHT * hand_card_scale;

    float hand_total_width = 0.0f;
    if (count > 0) hand_total_width = (count - 1) * card_spacing + draw_w_card;

    // posiciona as cartas
    float edge_padding = 30.0f; // padding
    float bottom_padding = 30.0f; // padding
    float pile_scale = 1.05f;
    float left_margin = edge_padding;
    float right_margin = edge_padding;
    float deck_x = left_margin;
    float discard_x = DISPLAY_BUFFER_WIDTH - DECK_WIDTH * pile_scale - right_margin;
    float pile_y = DISPLAY_BUFFER_HEIGHT - DECK_HEIGHT * pile_scale - bottom_padding;
    float hand_y = DISPLAY_BUFFER_HEIGHT - draw_h_card - bottom_padding;

    float central_left = deck_x + DECK_WIDTH;
    float central_right = discard_x;
    float central_width = central_right - central_left;
    float start_x = central_left + (central_width - hand_total_width) / 2.0f;

    if (hand_total_width > central_width && count > 1) {
        card_spacing = (central_width - draw_w_card) / (count - 1);
        if (card_spacing < 8.0f) card_spacing = 8.0f;
        hand_total_width = (count - 1) * card_spacing + draw_w_card;
        start_x = central_left + (central_width - hand_total_width) / 2.0f;
    }

    float deck_y = pile_y;
    if (r->deck_image) {
        int dw = al_get_bitmap_width(r->deck_image);
        int dh = al_get_bitmap_height(r->deck_image);
        al_draw_scaled_bitmap(r->deck_image, 0, 0, dw, dh, deck_x, deck_y, DECK_WIDTH * pile_scale, DECK_HEIGHT * pile_scale, 0);
    } else {
        RenderDeck(r, deck, (int)deck_x, (int)deck_y, 0);
    }

    ALLEGRO_BITMAP* player_bmp = r->player_image;
    int pw = al_get_bitmap_width(player_bmp);
    int ph = al_get_bitmap_height(player_bmp);
    float player_max_w = CARD_WIDTH * 0.9f;
    Combat* c = r->combat;
    int n = (c->num_enemies > 0) ? c->num_enemies : 1;
    float right_area_x = DISPLAY_BUFFER_WIDTH * 0.55f;
    float right_area_w = DISPLAY_BUFFER_WIDTH - right_area_x - 40.0f;
    float spacing = right_area_w / (n + 1);
    float enemy_allowed_w = spacing * 0.8f;
    player_max_w = enemy_allowed_w;
    float pscale = (float)player_max_w / (float)pw;
    if (pscale > 0.6f) pscale = 0.6f;
    if (pscale < 0.15f) pscale = 0.15f;
    float player_draw_w = pw * pscale;
    float player_draw_h = ph * pscale;

    float player_extra = 1.12f;
    player_draw_w *= player_extra;
    player_draw_h *= player_extra;

    float player_x = deck_x + DECK_WIDTH / 2.0f - player_draw_w / 2.0f;
    float player_y = deck_y - player_draw_h - 16.0f;

    r->player_draw_x = player_x;
    r->player_draw_y = player_y;
    r->player_draw_w = player_draw_w;
    r->player_draw_h = player_draw_h;

    // centraliza a mão
    for (int i = 0; i < count; i++) {
        int selected = (i == r->combat->selected_card) && !(r->combat->intro_mode || r->combat->victory_mode || r->combat->game_over_mode);
        float x = start_x + i * card_spacing;
        RenderCard(r, &deck->hand.cards[i], (int)x, (int)hand_y, selected);
    }

    // desenha pilha de descarte
    discard_x = DISPLAY_BUFFER_WIDTH - DECK_WIDTH - right_margin;
    float discard_y = deck_y;
    if (r->discard_image) {
        int dw = al_get_bitmap_width(r->discard_image);
        int dh = al_get_bitmap_height(r->discard_image);
        al_draw_scaled_bitmap(r->discard_image, 0, 0, dw, dh, discard_x, discard_y, DECK_WIDTH * pile_scale, DECK_HEIGHT * pile_scale, 0);
    } else {
        al_draw_filled_rounded_rectangle(discard_x, discard_y, discard_x + DECK_WIDTH * pile_scale, discard_y + DECK_HEIGHT * pile_scale, 10, 0, al_map_rgb(120,120,120));
    }
    
    // gambiarra em sua mais pura essência (não consegui desenhar o counter do lado do deck e da pilha de descarte)
    char draw_count[16];
    sprintf(draw_count, "Deck: %d", deck->draw_pile.size);
    float left_text_x = start_x - 80.0f;
    float text_y = hand_y + draw_h_card / 2.0f;
    DrawScaledText(r->font, al_map_rgb(255,255,255), left_text_x / 2.2, text_y / 2.2, 2.2, 2.2, ALLEGRO_ALIGN_CENTER, draw_count);

    char discard_count[16];
    sprintf(discard_count, "Desc: %d", deck->discard_pile.size);
    float right_text_x = start_x + hand_total_width + 80.0f;
    DrawScaledText(r->font, al_map_rgb(255,255,255), right_text_x / 2.2, text_y / 2.2, 2.2, 2.2, ALLEGRO_ALIGN_CENTER, discard_count);

}



void RenderEnemies(Renderer* r) {
    Combat* c = r->combat;

    // evita renderização em telas que não de combate
    if (c->intro_mode || c->victory_mode || c->game_over_mode) return;

    // posiciona inimigos
    float right_area_x = DISPLAY_BUFFER_WIDTH * 0.55f;
    float right_area_w = DISPLAY_BUFFER_WIDTH - right_area_x - 40.0f; /* margin */
    float y = DISPLAY_BUFFER_HEIGHT * 0.25f;
    int n = (c->num_enemies > 0) ? c->num_enemies : 1;
    float spacing = right_area_w / (n + 1);
    for (int i = 0; i < c->num_enemies; i++) {
        Enemy* e = &c->enemies[i];

        // não desenha inimigos mortos
        if (e->hp <= 0) {
            continue;
        }

        ALLEGRO_BITMAP* sprite =
            (e->is_boss) ? r->boss :
            (e->type == ENEMY_WEAK) ? r->enemy_weak :
            (e->type == ENEMY_STRONG) ? r->enemy_strong :
            r->boss;
            
        int selected = (i == c->selected_enemy);

        int w = al_get_bitmap_width(sprite);
        int h = al_get_bitmap_height(sprite);

        float max_allowed_w = spacing * 0.8f;
        float scale = max_allowed_w / (float)w;
        if (scale > 0.9f) scale = 0.9f;
        if (scale < 0.35f) scale = 0.35f;
        float draw_w = w * scale;
        float draw_h = h * scale;

        float x = right_area_x + (i + 1) * spacing - draw_w/2.0f;

        // animação de seleção
        float draw_y = y;
        if (selected) draw_y -= 20.0f;

        if (e->highlight_frames > 0) {
            al_draw_tinted_scaled_bitmap(sprite, al_map_rgba(255,0,0,128), 0, 0, w, h, x, draw_y, draw_w, draw_h, 0);
        } else {
            al_draw_scaled_bitmap(sprite, 0, 0, w, h, x, draw_y, draw_w, draw_h, 0);
        }

        if (selected && sprite != NULL && !(r->combat->intro_mode || r->combat->victory_mode || r->combat->game_over_mode)) {
            al_draw_rounded_rectangle(
                x - 10, draw_y - 10, x + draw_w + 10, draw_y + draw_h + 10,
                12, 12, al_map_rgb(255, 255, 0), 4
            );
        }

        RenderHealthBarFull(
            x, x + draw_w, draw_y + draw_h + 10, e->hp, e->max_hp,
            e->block, r->font
        );
    }
}

void RenderEnergy(Renderer* r) {
    Player* p = &r->combat->player;

    char txt[32];
    sprintf(txt, "%d / %d", p->energy, p->max_energy);

    if (r->player_draw_w > 0.0f) {
        float x = r->player_draw_x + r->player_draw_w / 2.0f;
        float y = r->player_draw_y - 20.0f;
        DrawScaledText(r->font, al_map_rgb(255,255,0), x, y, 2.5, 2.5, ALLEGRO_ALIGN_CENTER, txt);
        
        // mostra dano extra acima da cabeça do jogador
        if (p->bonus_damage > 0) {
            char dmg_txt[32];
            sprintf(dmg_txt, "+%d", p->bonus_damage);
            float dmg_y = r->player_draw_y - 60.0f;
            DrawScaledText(r->font, al_map_rgb(255,255,255), x / 2.2, dmg_y / 2.2, 2.2, 2.2, ALLEGRO_ALIGN_CENTER, dmg_txt);
        }
    } else {
        DrawScaledText(r->font, al_map_rgb(255,255,0), 20, 24, 2.5, 2.5, ALLEGRO_ALIGN_LEFT, txt);
    }
}

// desenha as informações extras sobre os inimigos (ação e escudo)
void RenderEnemyOverlays(Renderer* r) {
    if (!r || !r->combat) return;
    Combat* c = r->combat;
    // garante renderização apenas em combate
    if (c->intro_mode || c->victory_mode || c->game_over_mode) return;

    float right_area_x = DISPLAY_BUFFER_WIDTH * 0.55f;
    float right_area_w = DISPLAY_BUFFER_WIDTH - right_area_x - 40.0f;
    float y = DISPLAY_BUFFER_HEIGHT * 0.25f;
    int n = (c->num_enemies > 0) ? c->num_enemies : 1;
    float spacing = right_area_w / (n + 1);

    for (int i = 0; i < c->num_enemies; i++) {
        Enemy* e = &c->enemies[i];
        if (e->hp <= 0) continue;

        ALLEGRO_BITMAP* sprite =
            (e->type == ENEMY_WEAK) ? r->enemy_weak :
            (e->type == ENEMY_STRONG) ? r->enemy_strong :
            r->boss;
        int w = al_get_bitmap_width(sprite);
        float max_allowed_w = spacing * 0.8f;
        float scale = max_allowed_w / (float)w;
        if (scale > 0.9f) scale = 0.9f;
        if (scale < 0.35f) scale = 0.35f;
        float draw_w = w * scale;
        float x = right_area_x + (i + 1) * spacing - draw_w/2.0f;
        float draw_y = y;
        if (i == c->selected_enemy) draw_y -= 20.0f;

        // ícone da ação seguinte
        EnemyAction* a = &e->actions[e->current_action];
        ALLEGRO_BITMAP* icon = (a->type == ACTION_ATTACK ? r->icon_attack : r->icon_block);
        if (icon) {
            int iw = al_get_bitmap_width(icon);
            int ih = al_get_bitmap_height(icon);

            float icon_x = x + draw_w/2 - 30;
            float icon_y = draw_y - ih - 10;
            al_draw_scaled_bitmap(icon, 0, 0, iw, ih, icon_x, icon_y, 60, 60, 0);
            // borda amarela
            al_draw_rectangle(icon_x, icon_y, icon_x + 60, icon_y + 60, al_map_rgb(255,255,0), 2);
            
            // escudo do inimigo
            if (e->block > 0) {
                char blkbuf[32];
                sprintf(blkbuf, "%d", e->block);
                const float b_xscale = 1.9f;
                const float b_yscale = 1.9f;
                float bx = (icon_x - 45.0f) / b_xscale;
                float by = (icon_y + 30.0f - (al_get_font_line_height(r->font) / 2.0f)) / b_yscale;
                DrawScaledText(r->font, al_map_rgb(255,255,255), bx, by, b_xscale, b_yscale, ALLEGRO_ALIGN_RIGHT, blkbuf);
            }
        } else {
            // fallback - ícone vermelho
            float px1 = x + draw_w/2 - 30;
            float py1 = draw_y - 50;
            float px2 = x + draw_w/2 + 30;
            float py2 = draw_y - 10;
            al_draw_filled_rectangle(px1, py1, px2, py2, al_map_rgba(255, 0, 0, 180));
        }
    }
}

// desenha informações extras sobre os inimigos no backbuffer para evitar flickering
void DrawBackbufferOverlays(struct Renderer* r) {
    if (!r || !r->combat || !r->display) return;
    Combat* c = r->combat;

    // garante renderização apenas em combate
    if (c->intro_mode || c->victory_mode || c->game_over_mode) return;

    float sx = DISPLAY_WIDTH / (float)DISPLAY_BUFFER_WIDTH;
    float sy = DISPLAY_HEIGHT / (float)DISPLAY_BUFFER_HEIGHT;

    // mesmo posicionamento para desenhar os overlays
    float right_area_x = DISPLAY_BUFFER_WIDTH * 0.55f;
    float right_area_w = DISPLAY_BUFFER_WIDTH - right_area_x - 40.0f;
    float y = DISPLAY_BUFFER_HEIGHT * 0.25f;
    int n = (c->num_enemies > 0) ? c->num_enemies : 1;
    float spacing = right_area_w / (n + 1);

    for (int i = 0; i < c->num_enemies; i++) {
        Enemy* e = &c->enemies[i];
        if (e->hp <= 0) continue;

        ALLEGRO_BITMAP* sprite =
            (e->type == ENEMY_WEAK) ? r->enemy_weak :
            (e->type == ENEMY_STRONG) ? r->enemy_strong :
            r->boss;
        int w = al_get_bitmap_width(sprite);
        float max_allowed_w = spacing * 0.8f;
        float scale = max_allowed_w / (float)w;
        if (scale > 0.9f) scale = 0.9f;
        if (scale < 0.35f) scale = 0.35f;
        float draw_w = w * scale;
        float bx = right_area_x + (i + 1) * spacing - draw_w/2.0f;
        float by = y;
        if (i == c->selected_enemy) by -= 20.0f;

        // ícone de ação
        EnemyAction* a = &e->actions[e->current_action];
        ALLEGRO_BITMAP* icon = (a->type == ACTION_ATTACK ? r->icon_attack : r->icon_block);
        float icon_x_buf = bx + draw_w/2 - 30;
        float icon_y_buf = by - 60;
        float icon_x = icon_x_buf * sx;
        float icon_y = icon_y_buf * sy;
        if (icon) {
            int iw = al_get_bitmap_width(icon);
            int ih = al_get_bitmap_height(icon);
            float icon_w = 60.0f * sx;
            float icon_h = 60.0f * sy;
            al_draw_scaled_bitmap(icon, 0, 0, iw, ih, icon_x, icon_y, icon_w, icon_h, 0);
            al_draw_rectangle(icon_x, icon_y, icon_x + icon_w, icon_y + icon_h, al_map_rgb(255,255,0), 2);
        } else {
            // fallback
            al_draw_filled_rectangle(icon_x, icon_y, icon_x + 60.0f*sx, icon_y + 60.0f*sy, al_map_rgba(255,0,0,180));
        }
    }

    // highlight da carta selecionada
    Deck* deck = &r->combat->player.deck;
    int count = deck->hand.size;
    if (count > 0) {
        float hand_card_scale = 0.95f;
        float card_spacing = CARD_WIDTH * hand_card_scale + 8.0f;
        float draw_w_card = CARD_WIDTH * hand_card_scale;
        float draw_h_card = CARD_HEIGHT * hand_card_scale;
        float hand_total_width = (count - 1) * card_spacing + draw_w_card;

        float edge_padding = 30.0f;
        float bottom_padding = 30.0f;
        float pile_scale = 1.05f;
        float left_margin = edge_padding;
        float right_margin = edge_padding;
        float deck_x = left_margin;
        float discard_x = DISPLAY_BUFFER_WIDTH - DECK_WIDTH * pile_scale - right_margin;
        float hand_y = DISPLAY_BUFFER_HEIGHT - draw_h_card - bottom_padding;

        float central_left = deck_x + DECK_WIDTH;
        float central_right = discard_x;
        float central_width = central_right - central_left;
        float start_x = central_left + (central_width - hand_total_width) / 2.0f;

        if (hand_total_width > central_width && count > 1) {
            card_spacing = (central_width - draw_w_card) / (count - 1);
            if (card_spacing < 8.0f) card_spacing = 8.0f;
            hand_total_width = (count - 1) * card_spacing + draw_w_card;
            start_x = central_left + (central_width - hand_total_width) / 2.0f;
        }
        int sel = r->combat->selected_card;
        if (sel >= 0 && sel < count) {
            float card_x_buf = start_x + sel * card_spacing;
            float card_y_buf = hand_y - ( (sel == r->combat->selected_card) ? 20.0f : 0.0f );
            float card_w_buf = CARD_WIDTH * hand_card_scale;
            float card_h_buf = CARD_HEIGHT * hand_card_scale;
            float cx = card_x_buf * sx;
            float cy = card_y_buf * sy;
            float cw = card_w_buf * sx;
            float ch = card_h_buf * sy;
            al_draw_rounded_rectangle(cx - 4, cy - 4, cx + cw + 4, cy + ch + 4, 12, 12, al_map_rgb(255,255,0), 4);
        }
    }
}

    void Render(Renderer* renderer) {
    if (!renderer || !renderer->display || !renderer->display_buffer || !renderer->combat) {
        // fallback para erro de renderização
        exit(1);
    }

    al_set_target_bitmap(renderer->display_buffer);
    al_clear_to_color(al_map_rgb(0, 0, 0));

    
    RenderBackground(renderer);
    RenderEnemies(renderer);
    RenderPlayerHand(renderer);
    RenderPlayer(renderer);
    RenderEnergy(renderer);

    // mostra número do turno
    if (!renderer->combat->intro_mode && !renderer->combat->victory_mode && !renderer->combat->game_over_mode) {
        RenderTurnNumber(renderer, renderer->combat->turn_number, 11);
    }

    // desenha overlays
    if (renderer->combat->intro_mode) {
        al_draw_filled_rectangle(0, 0, DISPLAY_BUFFER_WIDTH, DISPLAY_BUFFER_HEIGHT, al_map_rgb(0, 0, 0));
        DrawLargeCenteredText(renderer->font, al_map_rgb(255, 255, 255), DISPLAY_BUFFER_WIDTH/2, DISPLAY_BUFFER_HEIGHT/2 - 60, 4.0, "BEM VINDO A AVENTURA!");
        DrawLargeCenteredText(renderer->font, al_map_rgb(180, 180, 180), DISPLAY_BUFFER_WIDTH/2, DISPLAY_BUFFER_HEIGHT/2, 3.0, "Se vingue das casas de aposta!");
        DrawLargeCenteredText(renderer->font, al_map_rgb(200, 200, 200), DISPLAY_BUFFER_WIDTH/2, DISPLAY_BUFFER_HEIGHT/2 + 60, 2.5, "Pressione Z para comecar!");
    } else if (renderer->combat->victory_mode) {
        al_draw_filled_rectangle(0, 0, DISPLAY_BUFFER_WIDTH, DISPLAY_BUFFER_HEIGHT, al_map_rgb(0,0,0));
        DrawLargeCenteredText(renderer->font, al_map_rgb(255,255,0), DISPLAY_BUFFER_WIDTH/2, DISPLAY_BUFFER_HEIGHT/2 - 60, 4.0, "VOCE GANHOU!");
        DrawLargeCenteredText(renderer->font, al_map_rgb(180, 180, 180), DISPLAY_BUFFER_WIDTH/2, DISPLAY_BUFFER_HEIGHT/2, 3.0, "Voce conseguiu sua vinganca!");
        DrawLargeCenteredText(renderer->font, al_map_rgb(200, 200, 200), DISPLAY_BUFFER_WIDTH/2, DISPLAY_BUFFER_HEIGHT/2 + 60, 2.5, "Pressione Z ou Q para sair, R para jogar novamente");
    } else if (renderer->combat->game_over_mode) {
        al_draw_filled_rectangle(0, 0, DISPLAY_BUFFER_WIDTH, DISPLAY_BUFFER_HEIGHT, al_map_rgb(0,0,0));
        DrawLargeCenteredText(renderer->font, al_map_rgb(255,50,50), DISPLAY_BUFFER_WIDTH/2, DISPLAY_BUFFER_HEIGHT/2 - 60, 4.0, "FIM DE JOGO!");
        DrawLargeCenteredText(renderer->font, al_map_rgb(180, 180, 180), DISPLAY_BUFFER_WIDTH/2, DISPLAY_BUFFER_HEIGHT/2, 3.0, "Tentar outra vez?");
        DrawLargeCenteredText(renderer->font, al_map_rgb(200, 200, 200), DISPLAY_BUFFER_WIDTH/2, DISPLAY_BUFFER_HEIGHT/2 + 60, 2.5, "Pressione R para recomecar, Z ou Q para sair");
    }

    // copia buffer com escala para o backbuffer
    al_set_target_backbuffer(renderer->display);
    al_draw_scaled_bitmap(renderer->display_buffer, 0, 0, DISPLAY_BUFFER_WIDTH, DISPLAY_BUFFER_HEIGHT, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0);

    DrawBackbufferOverlays(renderer);

    al_flip_display();
}

void ClearRenderer(Renderer* r) {
    if (r->background) al_destroy_bitmap(r->background);

    if (r->cards.card_images[CARD_ATTACK])  al_destroy_bitmap(r->cards.card_images[CARD_ATTACK]);
    if (r->cards.card_images[CARD_DEFENSE]) al_destroy_bitmap(r->cards.card_images[CARD_DEFENSE]);
    if (r->cards.card_images[CARD_SPECIAL]) al_destroy_bitmap(r->cards.card_images[CARD_SPECIAL]);
	if (r->cards.card_images[CARD_HEAL]) al_destroy_bitmap(r->cards.card_images[CARD_HEAL]);
    if (r->cards.card_images[CARD_BUFF_DAMAGE]) al_destroy_bitmap(r->cards.card_images[CARD_BUFF_DAMAGE]);

    if (r->deck_image)    al_destroy_bitmap(r->deck_image);
    if (r->discard_image) al_destroy_bitmap(r->discard_image);

    if (r->player_image) al_destroy_bitmap(r->player_image);

    if (r->enemy_weak)   al_destroy_bitmap(r->enemy_weak);
    if (r->enemy_strong) al_destroy_bitmap(r->enemy_strong);
    if (r->boss)         al_destroy_bitmap(r->boss);
    
    if (r->icon_attack) al_destroy_bitmap(r->icon_attack);
	if (r->icon_block)  al_destroy_bitmap(r->icon_block);

    if (r->display_buffer) al_destroy_bitmap(r->display_buffer);
    if (r->display)        al_destroy_display(r->display);
    if (r->font)           al_destroy_font(r->font);

	if (r->music_normal)   al_destroy_audio_stream(r->music_normal);
	if (r->music_boss)     al_destroy_audio_stream(r->music_boss);
	if (r->music_gameover) al_destroy_audio_stream(r->music_gameover);
}

void RenderTurnNumber(Renderer* r, int turn_number, int total_turns) {
    char buf[32];
    sprintf(buf, "Turno %d / %d", turn_number, total_turns);
    
    const float tn_xscale = 2.8f;
    const float tn_yscale = 2.8f;
    float t_x = (DISPLAY_BUFFER_WIDTH / 2.0f) / tn_xscale;
    float t_y = 40.0f / tn_yscale;
    DrawCenteredScaledText(r->font, al_map_rgb(255, 255, 0), t_x, t_y, tn_xscale, tn_yscale, buf);
}

void PlayMusic(Renderer* r, ALLEGRO_AUDIO_STREAM* stream, float vol) {
    if (!al_is_audio_installed() || !stream) {
        return;
    }
    
    // se já está tocando a mesma música, não faça nada
    if (r->current_music == stream) {
        return;
    }

    // para a música atual se houver
    if (r->current_music) {
        al_set_audio_stream_playing(r->current_music, false);
    }

    // configura e toca a nova música
    al_set_audio_stream_gain(stream, vol);
    al_set_audio_stream_playmode(stream, ALLEGRO_PLAYMODE_LOOP);
    al_rewind_audio_stream(stream);
    al_set_audio_stream_playing(stream, true);
    
    r->current_music = stream;
}

 // o espaçamento está inconsistente devido ao devkit não dar tab, e sim dois espaços. Deu preguiça de padronizar tudo.