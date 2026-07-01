#ifndef CARD_H
#define CARD_H

typedef enum {
    CARD_ATTACK,
    CARD_DEFENSE,
    CARD_SPECIAL,
    CARD_HEAL,
    CARD_BUFF_DAMAGE,
    CARD_DEBUFF_DEFENSE,
    CARD_ATTACK_HEAL,
    CARD_JOKER
} CardType;

typedef struct {
    CardType type;
    int cost;
    int effect;
    char name[32];
} Card;

int generate_random_attack(int cost);
int generate_random_defense(int cost);
int generate_random_heal(int cost);
int generate_random_buff_damage(int cost);
Card generate_special();

#endif
