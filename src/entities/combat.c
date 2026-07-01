#include "renderer.h"
#include "combat.h"
#include "deck.h"
#include "enemy.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <allegro5/allegro5.h>
#include <allegro5/allegro.h>
#include <allegro5/keycodes.h>
#include <allegro5/keyboard.h>

#define HIGHLIGHT_FRAMES 5

// registra e valida o index do inimigo selecionado
static void set_selected_enemy(Combat* c, int idx) {
    if (!c) return;
    int old = c->selected_enemy;
    if (c->num_enemies <= 0) idx = 0;
    if (idx < 0) idx = 0;
    if (idx >= c->num_enemies) idx = c->num_enemies - 1;
    if (idx < 0) idx = 0; // proteção caso num_enemies seja 0
    if (old != idx) {
        c->selected_enemy = idx;
    }
}

// retorna o index do próximo inimigo vivo
static int find_next_alive(Combat* c, int start) {
    if (c->num_enemies <= 0) return -1;
    for (int off = 1; off < c->num_enemies; off++) {
        int idx = (start + off) % c->num_enemies;
        if (c->enemies[idx].hp > 0) return idx;
    }
    return -1;
}

// retorna o index do inimigo vivo anterior (para garantir funcionamento)
static int find_prev_alive(Combat* c, int start) {
    if (c->num_enemies <= 0) return -1;
    for (int off = 1; off < c->num_enemies; off++) {
        int idx = (start - off) % c->num_enemies;
        if (idx < 0) idx += c->num_enemies;
        if (c->enemies[idx].hp > 0) return idx;
    }
    return -1;
}

/* garante que c->selected_enemy aponte para um inimigo vivo. se não houver nenhum vivo,
define para 0 */
static void ensure_selected_enemy_alive(Combat* c) {
    if (c->num_enemies <= 0) { c->selected_enemy = 0; return; }
    if (c->selected_enemy < 0 || c->selected_enemy >= c->num_enemies) { set_selected_enemy(c, 0); return; }
    if (c->enemies[c->selected_enemy].hp > 0) return; // já está em inimigo vivo
    int next = find_next_alive(c, c->selected_enemy);
    if (next >= 0) {
        set_selected_enemy(c, next);
    } else {
        set_selected_enemy(c, 0);
    }
}

void setup_enemies_for_turn(Combat* c, int turn_number) {
    c->num_enemies = 2;

    if (turn_number == 11) {
        // Boss
        c->boss_spawned = 1;
        c->enemies[0] = generate_boss();
        c->enemies[1] = generate_enemy();
    } else {
        // 2 inimigos, 5% de chance de um ou *mais* serem fortes (asterísco pois isso não está nas especificações do tp)
        for (int i = 0; i < 2; i++) {
            c->enemies[i] = generate_enemy();
        }
    }

    for (int i = 0; i < c->num_enemies; i++) {
        c->enemies[i].current_action = 0;
        c->enemies[i].block = 0;
        c->enemies[i].highlight_frames = 0;
        c->enemies[i].vulnerable = 0;
    }
}

void start_combat(Combat* c, Renderer* r) {
    PlayMusic(r, r->music_normal, 0.8);
    c->intro_mode = 1;
	c->victory_mode = 0;
	c->game_over_mode = 0;
	c->turn_number = 1;
    c->boss_spawned = 0;
    c->boss_music_started = 0;

    init_player(&c->player);
    c->turn_number = 1;

    setup_enemies_for_turn(c, c->turn_number);

    c->selected_card = 0;
    set_selected_enemy(c, 0);
    c->combat_over = 0;

    draw_cards(&c->player.deck, 5);
}

void player_turn(Combat* c, int card_index, int enemy_index) {
    if (card_index < 0 || card_index >= c->player.deck.hand.size) return;

    Card* card = &c->player.deck.hand.cards[card_index];
    if (card->cost > c->player.energy) return;

    // para cartas que precisam de alvo (ataque, debuff, ataque e cura)
    Enemy* target = NULL;
    if (card->type == CARD_ATTACK || card->type == CARD_DEBUFF_DEFENSE || card->type == CARD_ATTACK_HEAL || card->type == CARD_JOKER) {
        if (enemy_index < 0 || enemy_index >= c->num_enemies) return;
        target = &c->enemies[enemy_index];
        if (target->hp <= 0) return;
    }

    if (card->type == CARD_ATTACK) {
        if (!target) return;
        int damage = card->effect + c->player.bonus_damage;
        // aplica vulnerabilidade
        if (target->vulnerable) {
            damage = damage + (damage / 2);
        }
        int actual_damage = damage > target->block ? damage - target->block : 0;
        target->block -= (damage - actual_damage);
        if (target->block < 0) target->block = 0;
        target->hp -= actual_damage;
        if (target->hp < 0) target->hp = 0;

        target->highlight_frames = HIGHLIGHT_FRAMES;

        if (target->hp <= 0) {
            target->vulnerable = 0;
            int next = find_next_alive(c, enemy_index);
            if (next == -1) next = 0;
            set_selected_enemy(c, next);
        }
    } else if (card->type == CARD_DEFENSE) {
        c->player.block += card->effect;
    } else if (card->type == CARD_SPECIAL) {
        c->player.energy -= card->cost;
        play_card(&c->player.deck, card_index);
        discard_hand(&c->player.deck);
        draw_cards(&c->player.deck, 5);
        move_discard_into_draw(&c->player.deck);
        c->player.highlight_frames = HIGHLIGHT_FRAMES;

        if (c->selected_card >= c->player.deck.hand.size)
            c->selected_card = c->player.deck.hand.size - 1;
        if (c->selected_card < 0)
            c->selected_card = 0;

        return; // já lida com a lógica das cartas e da mão
    } else if (card->type == CARD_HEAL) {
        // efeito de cura
        c->player.hp += card->effect;
        if (c->player.hp > c->player.max_hp) c->player.hp = c->player.max_hp;
        c->player.highlight_frames = HIGHLIGHT_FRAMES;
    } else if (card->type == CARD_BUFF_DAMAGE) {
		c->player.bonus_damage += card->effect;
    } else if (card->type == CARD_DEBUFF_DEFENSE) {
        if (target) target->vulnerable = 1;
    } else if (card->type == CARD_ATTACK_HEAL) {
        if (!target) return;
        int damage = card->effect + c->player.bonus_damage;
        int actual_damage = damage > target->block ? damage - target->block : 0;
        target->block -= (damage - actual_damage);
        if (target->block < 0) target->block = 0;
        target->hp -= actual_damage;
        
        if (target->hp < 0) target->hp = 0;
        
        int current_hp = c->player.hp;
        c->player.hp += actual_damage;
        
        if (c->player.hp > c->player.max_hp) c->player.hp = c->player.max_hp;
        if (c->player.hp != current_hp) c->player.highlight_frames = HIGHLIGHT_FRAMES;
        
        if (target->hp <= 0) {
            target->vulnerable = 0;
            int next = find_next_alive(c, enemy_index);
            if (next == -1) next = 0;
            set_selected_enemy(c, next);
        }
	} else if (card->type == CARD_JOKER) {
    if (!target) return;
        // ataque e defesa (modificação normal)
        c->player.block += card->effect;
        int damage = card->effect + c->player.bonus_damage;
        // aplica vulnerabilidade
        if (target->vulnerable) {
            damage = damage + (damage / 2);
        }
        int actual_damage = damage > target->block ? damage - target->block : 0;
        target->block -= (damage - actual_damage);
        if (target->block < 0) target->block = 0;
        target->hp -= actual_damage;
        if (target->hp < 0) target->hp = 0;

        target->highlight_frames = HIGHLIGHT_FRAMES;

        if (target->hp <= 0) {
            target->vulnerable = 0;
            int next = find_next_alive(c, enemy_index);
            if (next == -1) next = 0;
            set_selected_enemy(c, next);
        }
    }

    c->player.energy -= card->cost;
    play_card(&c->player.deck, card_index);
    
    if (c->selected_card >= c->player.deck.hand.size)
    c->selected_card = c->player.deck.hand.size - 1;

	if (c->selected_card < 0)
		c->selected_card = 0;
    // garantir que a seleção de inimigo aponte para um vivo após o turno do jogador
    ensure_selected_enemy_alive(c);
}

void enemies_turn(Combat* c) {
    for (int i = 0; i < c->num_enemies; i++) {
        c->enemies[i].block = 0;
    }
    
    for (int i = 0; i < c->num_enemies; i++) {
        Enemy* e = &c->enemies[i];
        if (e->hp <= 0) continue;

        if (e->num_actions <= 0) {
            continue;
        }

        if (e->current_action < 0 || e->current_action >= e->num_actions) {
            e->current_action = 0;
        }

        EnemyAction* action = &e->actions[e->current_action];

        if (action->type == ACTION_ATTACK) {
            int dmg = action->value;
            int actual = dmg > c->player.block ? dmg - c->player.block : 0;
            c->player.block -= (dmg - actual);
            if (c->player.block < 0) c->player.block = 0;
            c->player.hp -= actual;
            if (c->player.hp < 0) c->player.hp = 0;

            c->player.highlight_frames = HIGHLIGHT_FRAMES;
        } else if (action->type == ACTION_BLOCK) {
            e->block += action->value;
        }

        if (e->num_actions > 0)
            e->current_action = (e->current_action + 1) % e->num_actions;
        else
            e->current_action = 0;
    }
}

void start_player_turn(Combat* c) {
    c->player.energy = c->player.max_energy;
    c->player.block = 0;

    discard_hand(&c->player.deck);
    draw_cards(&c->player.deck, 5);

    for (int i = 0; i < c->num_enemies; i++) {
        c->enemies[i].highlight_frames = 0;
    }

    ensure_selected_enemy_alive(c);
}

void update_combat(Combat* c, unsigned char* keys, Renderer* r) {
	// ---------- INTRO SCREEN ----------
	if (c->intro_mode) {
		if (keys[ALLEGRO_KEY_Z] & GAME_KEY_SEEN) {
			c->intro_mode = 0;
		}
		return;
	}

	// ---------- VICTORY SCREEN ----------
	if (c->victory_mode) {
        // Z / Q sai do jogo, R reinicia (como o game over)
        if (keys[ALLEGRO_KEY_Z] & GAME_KEY_SEEN) {
            exit(0);
        }
        if (keys[ALLEGRO_KEY_R] & GAME_KEY_SEEN) {
            start_combat(c, r);
            c->game_over_mode = 0;
            c->intro_mode = 0;
            c->victory_mode = 0;
            PlayMusic(r, r->music_normal, 0.8);
            return;
        }
        return;
	}
	// game over check
	if (c->game_over_mode) {
		if (keys[ALLEGRO_KEY_Z] & GAME_KEY_SEEN) {
			exit(0);
		}
		if (keys[ALLEGRO_KEY_R] & GAME_KEY_SEEN) {
			start_combat(c, r);
			c->game_over_mode = 0;
			c->intro_mode = 0;
			PlayMusic(r, r->music_normal, 0.8);
			return;
		}
		return;
	}
    if (c->combat_over) return;
    
    // checa se o player morreu antes de processar qualquer input
    if (c->player.hp <= 0) {
        c->game_over_mode = 1;
        PlayMusic(r, r->music_gameover, 0.7);
        return;
    }
    
    if (c->boss_spawned && !c->boss_music_started) {
		c->boss_music_started = 1;
		PlayMusic(r, r->music_boss, 0.9);
	}

    if (keys[ALLEGRO_KEY_LEFT] & GAME_KEY_SEEN) {
        if (c->selected_card > 0) c->selected_card--;
    }
    if (keys[ALLEGRO_KEY_RIGHT] & GAME_KEY_SEEN) {
        if (c->selected_card < c->player.deck.hand.size - 1) c->selected_card++;
    }

    if (keys[ALLEGRO_KEY_UP] & GAME_KEY_SEEN) {
        int prev = find_prev_alive(c, c->selected_enemy);
        if (prev >= 0) set_selected_enemy(c, prev);
    }
    if (keys[ALLEGRO_KEY_DOWN] & GAME_KEY_SEEN) {
        int next = find_next_alive(c, c->selected_enemy);
        if (next >= 0) set_selected_enemy(c, next);
    }

    if (keys[ALLEGRO_KEY_ENTER] & GAME_KEY_SEEN) {
        player_turn(c, c->selected_card, c->selected_enemy);
    }

    if (keys[ALLEGRO_KEY_ESCAPE] & GAME_KEY_SEEN) {
        enemies_turn(c);
        start_player_turn(c);
        c->selected_card = 0;
        // garantir seleção válida (aponta para um inimigo vivo, se houver)
        ensure_selected_enemy_alive(c);
    }
    
    if (keys[ALLEGRO_KEY_SPACE] & GAME_KEY_SEEN) {
        for (int i = 0; i < c->num_enemies; i++) {
			c->enemies[i].hp = 0;
		}
    }
    
    if (keys[ALLEGRO_KEY_X] & GAME_KEY_SEEN) {
        c->player.hp = 1;
    }

    if (c->player.highlight_frames > 0) c->player.highlight_frames--;
    for (int i = 0; i < c->num_enemies; i++) {
        if (c->enemies[i].highlight_frames > 0) c->enemies[i].highlight_frames--;
    }

    int all_enemies_dead = 1;
    int boss_dead = 0;
    for (int i = 0; i < c->num_enemies; i++) {
        if (c->enemies[i].hp > 0) all_enemies_dead = 0;
        if (c->enemies[i].is_boss && c->enemies[i].hp <= 0) boss_dead = 1;
    }
    if (boss_dead) {
		c->victory_mode = 1;
		return;
	}
    if (all_enemies_dead) {
        // prepara a próxima rodada
        c->turn_number++;
        setup_enemies_for_turn(c, c->turn_number);
        c->selected_card = 0;
        set_selected_enemy(c, 0);
        start_player_turn(c);
        c->combat_over = 0;
    }
}
