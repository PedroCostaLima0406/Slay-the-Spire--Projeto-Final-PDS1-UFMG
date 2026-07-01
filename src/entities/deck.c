#include "deck.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// versão adaptada do utils
void shuffle_card_array(CardArray* a) {
    if (a->size <= 1) return;

    int indices[MAX_CARDS];
    for (int i = 0; i < a->size && i < MAX_CARDS; i++) indices[i] = i;

    ShuffleArray(indices, a->size);

    Card temp[MAX_CARDS];
    for (int i = 0; i < a->size && i < MAX_CARDS; i++) temp[i] = a->cards[indices[i]];

    for (int i = 0; i < a->size && i < MAX_CARDS; i++) a->cards[i] = temp[i];
}


// tem 21 cartas por causa da modificação normal
void init_deck(Deck* d) {
    d->draw_pile.size = 0;
    d->discard_pile.size = 0;
    d->hand.size = 0;

    // 6 cartas de ataque
    for (int i = 0; i < 6; i++) {
        Card c;
        c.type = CARD_ATTACK;
        if (i == 5) {
			c.cost = 3;
			c.effect = generate_random_attack(c.cost);
		} else if (i >= 2) {
			c.cost = 2;
			c.effect = generate_random_attack(c.cost);
		} else if (i >= 1) {
			c.cost = 1;
			c.effect = generate_random_attack(c.cost);
		} else {
			c.cost = 0;
			c.effect = generate_random_attack(c.cost);
		}

        snprintf(c.name, sizeof(c.name), "Ataque %d", i + 1);
        if (d->draw_pile.size < MAX_CARDS)
            d->draw_pile.cards[d->draw_pile.size++] = c;
    }

    // 6 cartas de defesa
    for (int i = 0; i < 6; i++) {
        Card c;
        c.type = CARD_DEFENSE;
        if (i == 5) {
			c.cost = 3;
			c.effect = generate_random_defense(c.cost);
		} else if (i >= 2) {
			c.cost = 2;
			c.effect = generate_random_defense(c.cost);
		} else if (i >= 1) {
			c.cost = 1;
			c.effect = generate_random_defense(c.cost);
		} else {
			c.cost = 0;
			c.effect = generate_random_defense(c.cost);
		}

        snprintf(c.name, sizeof(c.name), "Defesa %d", i + 1);
        if (d->draw_pile.size < MAX_CARDS)
            d->draw_pile.cards[d->draw_pile.size++] = c;
    }

    // 2 cartas especiais
    for (int i = 0; i < 2; i++) {
        Card c = generate_special();
        snprintf(c.name, sizeof(c.name), "Especial %d", i + 1);
        if (d->draw_pile.size < MAX_CARDS)
            d->draw_pile.cards[d->draw_pile.size++] = c;
    }
    
    // 2 cartas cura
    for (int i = 0; i < 2; i++) {
        Card c;
        c.type = CARD_HEAL;
        if (i == 0) {
			c.cost = 1;
			c.effect = generate_random_heal(c.cost);
		} else {
			c.cost = 2;
			c.effect = generate_random_heal(c.cost);
		} 

        snprintf(c.name, sizeof(c.name), "Cura %d", i + 1);
        if (d->draw_pile.size < MAX_CARDS)
            d->draw_pile.cards[d->draw_pile.size++] = c;
    }

	// 2 cartas buff dano
	for (int i = 0; i < 2; i++) {
        Card c;
        c.type = CARD_BUFF_DAMAGE;
        if (i == 0) {
			c.cost = 1;
			c.effect = generate_random_buff_damage(c.cost);
		} else {
			c.cost = 2;
			c.effect = generate_random_buff_damage(c.cost);
		} 

        snprintf(c.name, sizeof(c.name), "Dano Bônus %d", i + 1);
        if (d->draw_pile.size < MAX_CARDS)
            d->draw_pile.cards[d->draw_pile.size++] = c;
    }
    
    // 1 carta debuff defesa (em um loop só para fins de padronização)
    for (int i = 0; i < 1; i++) {
        Card c;
        c.type = CARD_DEBUFF_DEFENSE;
		c.cost = 1;
		c.effect = 0;

        snprintf(c.name, sizeof(c.name), "Reduz Defesa %d", i + 1);
        if (d->draw_pile.size < MAX_CARDS)
            d->draw_pile.cards[d->draw_pile.size++] = c;
    }
    
    // 1 carta ataque e cura (em um loop só para fins de padronização)
    for (int i = 0; i < 1; i++) {
        Card c;
        c.type = CARD_ATTACK_HEAL;
		c.cost = 2;
		c.effect = generate_random_attack(c.cost);

        snprintf(c.name, sizeof(c.name), "Ataque e Cura %d", i + 1);
        if (d->draw_pile.size < MAX_CARDS)
            d->draw_pile.cards[d->draw_pile.size++] = c;
    }

    // 1 carta coringa 
    for (int i = 0; i < 1; i++) {
        Card c;
        c.type = CARD_JOKER;
        c.cost = rand() % 4; // custo aleatório entre 0 e 3
        c.effect = generate_random_attack(c.cost);

        snprintf(c.name, sizeof(c.name), "Coringa %d", i + 1);
        if (d->draw_pile.size < MAX_CARDS)
            d->draw_pile.cards[d->draw_pile.size++] = c;
    }

    shuffle_card_array(&d->draw_pile);
}

void move_discard_into_draw(Deck* d) {
    for (int i = 0; i < d->discard_pile.size; i++) {
        if (d->draw_pile.size < MAX_CARDS) {
            d->draw_pile.cards[d->draw_pile.size++] = d->discard_pile.cards[i];
        } else {
            break;
        }
    }
    d->discard_pile.size = 0;
    shuffle_card_array(&d->draw_pile);
}

void draw_cards(Deck* d, int n) {
    for (int i = 0; i < n; i++) {
        if (d->draw_pile.size == 0) {
            move_discard_into_draw(d);
            if (d->draw_pile.size == 0) return;
        }

        if (d->hand.size < MAX_CARDS) {
            d->hand.cards[d->hand.size++] = d->draw_pile.cards[--d->draw_pile.size];
        } else {
            return;
        }
    }
}

void discard_card(Deck* d, int index) {
    if (index < 0 || index >= d->hand.size) return;
    if (d->discard_pile.size < MAX_CARDS) d->discard_pile.cards[d->discard_pile.size++] = d->hand.cards[index];
    for (int i = index; i < d->hand.size - 1; i++) d->hand.cards[i] = d->hand.cards[i + 1];
    d->hand.size--;
}

void discard_hand(Deck* d) {
    for (int i = 0; i < d->hand.size; i++) {
        if (d->discard_pile.size < MAX_CARDS)
            d->discard_pile.cards[d->discard_pile.size++] = d->hand.cards[i];
        else
            break;
    }

    d->hand.size = 0;
}

void play_card(Deck* deck, int index) {
    if (index < 0 || index >= deck->hand.size) return;

    Card c = deck->hand.cards[index];

    (void)c; // evita warining "unused variable" (sinceramente, não sei por que funciona, só sei que resolveu :/)

    if (deck->discard_pile.size < MAX_CARDS) deck->discard_pile.cards[deck->discard_pile.size++] = c;

    for (int i = index; i < deck->hand.size - 1; i++)
        deck->hand.cards[i] = deck->hand.cards[i + 1];
    deck->hand.size--;
}
