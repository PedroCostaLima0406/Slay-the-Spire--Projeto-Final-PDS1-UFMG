#include "enemy.h"
#include <stdlib.h>

Enemy generate_enemy() {
    Enemy e;

    // garante que o campo is_boss esteja inicializado (evita lixo quando não é boss)
    e.is_boss = 0;

    int strongChance = rand() % 100 < 5;
    e.type = strongChance ? ENEMY_STRONG : ENEMY_WEAK;

    if (e.type == ENEMY_WEAK) {
        e.max_hp = 10 + rand() % 21;
        int num_actions = 1 + rand() % 2;
        e.num_actions = num_actions;

        if(num_actions == 1) {
            e.actions[0].type = ACTION_ATTACK;
            e.actions[0].value = 3 + rand() % 6 + 2;
            e.hp = e.max_hp;
            e.block = 0;
            e.current_action = 0;

            e.highlight_frames = 0;
            e.vulnerable = 0;
            return e;
        }
        for (int i = 0; i < e.num_actions; i++) {
            int attackChance = rand() % 2;
            if (attackChance) {
                e.actions[i].type = ACTION_ATTACK;
                e.actions[i].value = 3 + rand() % 6 + 2;
            } else {
                e.actions[i].type = ACTION_BLOCK;
                e.actions[i].value = 3 + rand() % 6;
            }
        }
        if(e.actions[0].type != ACTION_ATTACK && e.actions[1].type != ACTION_ATTACK) {
            e.actions[0].type = ACTION_ATTACK;
            e.actions[0].value = 3 + rand() % 6 + 2;
        }
    } else {
        e.max_hp = 40 + rand() % 61;
        int num_actions = 2 + rand() % 2;
        e.num_actions = num_actions;

        int level1_used = 0;
        for (int i = 0; i < e.num_actions; i++) {
            int attackChance = rand() % 2;
            e.actions[i].type = attackChance ? ACTION_ATTACK : ACTION_BLOCK;

            int level = (rand() % 3) + 1; // 1–3
            if (level == 1 && level1_used) level++;

            if (level == 1) level1_used = 1;

            if (level == 1) e.actions[i].value = 5 + rand() % 6 + 5;
            if (level == 2) e.actions[i].value = 10 + rand() % 6 + 5;
            if (level == 3) e.actions[i].value = 15 + rand() % 16 + 5;
        }

        // garante que pelo menos uma ação seja de ataque (o for já garante no máximo uma ação de nível 1, mas não garante ataque)
        if(num_actions == 2) {
            if(e.actions[0].type != ACTION_ATTACK && e.actions[1].type != ACTION_ATTACK) {
                e.actions[0].type = ACTION_ATTACK;

                int level = (rand() % 2) + 2; // 2 ou 3 apenas

                if (level == 2) e.actions[0].value = 10 + rand() % 6 + 5;
                if (level == 3) e.actions[0].value = 15 + rand() % 16 + 5;
            }
        }
        else if(num_actions == 3) {
            if(e.actions[0].type != ACTION_ATTACK && e.actions[1].type != ACTION_ATTACK && e.actions[2].type != ACTION_ATTACK) {
                e.actions[0].type = ACTION_ATTACK;
                
                int level = (rand() % 2) + 2; // 2 ou 3 apenas

                if (level == 2) e.actions[0].value = 10 + rand() % 6 + 5;
                if (level == 3) e.actions[0].value = 15 + rand() % 16 + 5;
            }
        }
    }

    e.hp = e.max_hp;
    e.block = 0;
    e.current_action = 0;

	e.highlight_frames = 0;
	e.vulnerable = 0;
	
    return e;
}

Enemy generate_boss() {
    Enemy e;
    e.is_boss = 1;

    e.max_hp = 350;
    e.hp = e.max_hp;
    e.block = 0;

    e.num_actions = 3;

    e.actions[0].type = ACTION_ATTACK;
    e.actions[0].value = 25;

    e.actions[1].type = ACTION_BLOCK;
    e.actions[1].value = 15;

    e.actions[2].type = ACTION_ATTACK;
    e.actions[2].value = 30;

    e.current_action = 0;

	e.highlight_frames = 0;
	e.vulnerable = 0;
    return e;
}
