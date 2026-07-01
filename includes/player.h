#ifndef PLAYER_H
#define PLAYER_H

#include "deck.h"

typedef struct {
    int hp;
    int max_hp;
    int block;
    int energy;
    int bonus_damage;
    int max_energy;
    int highlight_frames;
    Deck deck;
} Player;

void init_player(Player* p);

#endif
