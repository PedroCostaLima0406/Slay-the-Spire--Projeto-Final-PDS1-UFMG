#ifndef ENEMY_H
#define ENEMY_H

typedef enum {
    ENEMY_WEAK,
    ENEMY_STRONG
} EnemyType;

typedef enum {
    ACTION_ATTACK,
    ACTION_BLOCK
} EnemyActionType;

typedef struct {
    EnemyActionType type;
    int value;
} EnemyAction;

typedef struct {
    EnemyType type;
    int hp, max_hp;
    int block;
	int highlight_frames;
    EnemyAction actions[3];
    int num_actions;
    int current_action;
    int is_boss;
    int vulnerable;
} Enemy;

Enemy generate_enemy();
Enemy generate_boss();

#endif

