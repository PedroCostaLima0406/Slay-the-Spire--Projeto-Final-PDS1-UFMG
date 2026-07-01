#include "player.h"

void init_player(Player* p) {
    p->max_hp = 100;
    p->hp = p->max_hp;
    p->block = 0;
    p->max_energy = 3;
    p->energy = p->max_energy;
    p->bonus_damage = 0;
    p->highlight_frames = 0;
    init_deck(&p->deck);
}
