#include "card.h"
#include <stdlib.h>

int generate_random_attack(int cost) {
	int effect = 0;
    if (cost == 0) effect = 1 + rand() % 5;
    else if (cost == 1) effect = 5 + rand() % 6;
    else if (cost == 2) effect = 10 + rand() % 6;
    else effect = 15 + rand() % 16;

    return effect;
}

int generate_random_defense(int cost) {
	int effect = 0;
    if (cost == 0) effect = 1 + rand() % 5;
    else if (cost == 1) effect = 5 + rand() % 6;
    else if (cost == 2) effect = 10 + rand() % 6;
    else effect = 15 + rand() % 16;

    return effect;
}

Card generate_special() {
    Card c;
    c.type = CARD_SPECIAL;
    c.cost = 0;
    c.effect = 0;
    return c;
}

int generate_random_heal(int cost) {
    int heal = 0;

    if (cost == 1) {
        heal = 8 + rand() % 5;// 8–12
    } else if (cost == 2) {
        heal = 15 + rand() % 6;// 15–20
    }

    return heal;
}

int generate_random_buff_damage(int cost) {
    if (cost == 1) return 2 + rand() % 3; // 2–4
    if (cost == 2) return 4 + rand() % 3; // 4–6
    return 0;
}
