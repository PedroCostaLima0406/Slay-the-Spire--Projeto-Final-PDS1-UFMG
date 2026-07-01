#ifndef DECK_H
#define DECK_H

#include "card.h"
#include "constants.h"

typedef struct {
    Card cards[MAX_CARDS];
    int size;
} CardArray;

typedef struct {
    CardArray draw_pile;
    CardArray discard_pile;
    CardArray hand;
} Deck;

void init_deck(Deck* d);
void shuffle_card_array(CardArray* a);
void move_discard_into_draw(Deck* deck);
void draw_cards(Deck* d, int n);
void discard_card(Deck* d, int index);
void discard_hand(Deck* d);
void play_card(Deck* deck, int index);

#endif

