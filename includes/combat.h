#ifndef COMBAT_H
#define COMBAT_H

#include "constants.h"
#include "player.h"
#include "enemy.h"
#include "deck.h"

typedef struct Renderer Renderer; // forward struct declaration (um precisa do outro, mas ambos se referenciam)

typedef struct {
    Player player;
    Enemy enemies[2];
    int num_enemies;

    int selected_card;
    int selected_enemy;
    int boss_spawned;
    int boss_music_started;

	int turn_number;
    int combat_over;
    
    int intro_mode;
    int victory_mode;
    int game_over_mode;
} Combat;

void setup_enemies_for_turn(Combat* c, int turn_number);
void start_combat(Combat* c, Renderer* r);
void player_turn(Combat* c, int card_index, int enemy_index);
void start_player_turn(Combat* c);
void enemies_turn(Combat* c);
void update_combat(Combat* c, unsigned char* keys, Renderer* r);

#endif
