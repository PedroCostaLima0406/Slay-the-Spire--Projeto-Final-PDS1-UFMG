#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "constants.h"
#include "combat.h"
#include "renderer.h"
#include "utils.h"

static void DebugPrintAssetBaseDir(const char* strategy, const char* dir) {
#ifndef NDEBUG
    fprintf(stderr, "[debug] asset base dir (%s): %s\n", strategy, dir ? dir : "<unknown>");
#else
    (void)strategy;
    (void)dir;
#endif
}

static void SetWorkingDirectoryForAssets(void) {
    ALLEGRO_PATH* exe_path = al_get_standard_path(ALLEGRO_EXENAME_PATH);
    if (!exe_path) return;

    // Candidate 1: executable directory contains assets/.
    al_set_path_filename(exe_path, NULL);

    ALLEGRO_PATH* assets_in_exe_dir = al_clone_path(exe_path);
    al_append_path_component(assets_in_exe_dir, "assets");

    if (al_filename_exists(al_path_cstr(assets_in_exe_dir, ALLEGRO_NATIVE_PATH_SEP))) {
        al_change_directory(al_path_cstr(exe_path, ALLEGRO_NATIVE_PATH_SEP));
        DebugPrintAssetBaseDir("exe_dir", al_path_cstr(exe_path, ALLEGRO_NATIVE_PATH_SEP));
        al_destroy_path(assets_in_exe_dir);
        al_destroy_path(exe_path);
        return;
    }

    // Candidate 2: parent directory of executable contains assets/ (e.g. build/game.exe).
    ALLEGRO_PATH* parent_dir = al_clone_path(exe_path);
    if (al_get_path_num_components(parent_dir) > 0) {
        al_remove_path_component(parent_dir, -1);
    }

    ALLEGRO_PATH* assets_in_parent = al_clone_path(parent_dir);
    al_append_path_component(assets_in_parent, "assets");

    if (al_filename_exists(al_path_cstr(assets_in_parent, ALLEGRO_NATIVE_PATH_SEP))) {
        al_change_directory(al_path_cstr(parent_dir, ALLEGRO_NATIVE_PATH_SEP));
        DebugPrintAssetBaseDir("parent_of_exe_dir", al_path_cstr(parent_dir, ALLEGRO_NATIVE_PATH_SEP));
    } else {
        char* cwd = al_get_current_directory();
        DebugPrintAssetBaseDir("unchanged_cwd", cwd);
        al_free(cwd);
    }

    al_destroy_path(assets_in_parent);
    al_destroy_path(parent_dir);
    al_destroy_path(assets_in_exe_dir);
    al_destroy_path(exe_path);
}

static int FirstAliveEnemy(const Combat* c) {
    for (int i = 0; i < c->num_enemies; i++) {
        if (c->enemies[i].hp > 0) return i;
    }
    return -1;
}

static bool BossDead(const Combat* c) {
    for (int i = 0; i < c->num_enemies; i++) {
        if (c->enemies[i].is_boss && c->enemies[i].hp <= 0) return true;
    }
    return false;
}

static bool AllEnemiesDead(const Combat* c) {
    for (int i = 0; i < c->num_enemies; i++) {
        if (c->enemies[i].hp > 0) return false;
    }
    return true;
}

static bool CardNeedsTarget(CardType type) {
    return type == CARD_ATTACK || type == CARD_DEBUFF_DEFENSE ||
           type == CARD_ATTACK_HEAL || type == CARD_JOKER;
}

static bool ValidateCombatState(const Combat* c) {
    if (c->player.max_hp <= 0) return false;
    if (c->player.hp < 0 || c->player.hp > c->player.max_hp) return false;
    if (c->player.energy < 0 || c->player.energy > c->player.max_energy) return false;
    if (c->player.deck.draw_pile.size < 0 || c->player.deck.draw_pile.size > MAX_CARDS) return false;
    if (c->player.deck.discard_pile.size < 0 || c->player.deck.discard_pile.size > MAX_CARDS) return false;
    if (c->player.deck.hand.size < 0 || c->player.deck.hand.size > MAX_CARDS) return false;
    if (c->num_enemies < 0 || c->num_enemies > 2) return false;
    if (c->num_enemies > 0 && (c->selected_enemy < 0 || c->selected_enemy >= c->num_enemies)) return false;
    if (c->turn_number <= 0) return false;

    for (int i = 0; i < c->num_enemies; i++) {
        if (c->enemies[i].max_hp <= 0) return false;
        if (c->enemies[i].hp < 0 || c->enemies[i].hp > c->enemies[i].max_hp) return false;
    }

    return true;
}

static void InitCombatForSelfTest(Combat* c) {
    c->intro_mode = 0;
    c->victory_mode = 0;
    c->game_over_mode = 0;
    c->turn_number = 1;
    c->boss_spawned = 0;
    c->boss_music_started = 0;
    c->combat_over = 0;

    init_player(&c->player);
    setup_enemies_for_turn(c, c->turn_number);
    c->selected_card = 0;
    c->selected_enemy = 0;
    draw_cards(&c->player.deck, 5);
}

static int RunSelfTest(int rounds) {
    int completed = 0;
    int losses = 0;
    int failures = 0;

    for (int seed = 1; seed <= rounds; seed++) {
        srand((unsigned int)seed);

        Combat c = {0};
        InitCombatForSelfTest(&c);

        bool done = false;
        bool won = false;

        for (int steps = 0; steps < 4000 && !done; steps++) {
            if (!ValidateCombatState(&c)) {
                printf("SELFTEST FAIL: invalid state at seed=%d step=%d\n", seed, steps);
                failures++;
                done = true;
                break;
            }

            if (c.player.hp <= 0) {
                losses++;
                done = true;
                break;
            }

            if (BossDead(&c)) {
                won = true;
                done = true;
                break;
            }

            if (AllEnemiesDead(&c)) {
                c.turn_number++;
                setup_enemies_for_turn(&c, c.turn_number);
                c.selected_card = 0;
                c.selected_enemy = 0;
                start_player_turn(&c);
                continue;
            }

            int enemy_index = FirstAliveEnemy(&c);
            if (enemy_index >= 0) c.selected_enemy = enemy_index;

            int card_index = -1;
            for (int i = 0; i < c.player.deck.hand.size; i++) {
                Card* card = &c.player.deck.hand.cards[i];
                if (card->cost > c.player.energy) continue;
                if (CardNeedsTarget(card->type) && enemy_index < 0) continue;
                card_index = i;
                break;
            }

            if (card_index >= 0) {
                player_turn(&c, card_index, c.selected_enemy);
            } else {
                enemies_turn(&c);
                start_player_turn(&c);
                c.selected_card = 0;
            }
        }

        if (!done) {
            printf("SELFTEST FAIL: timeout at seed=%d\n", seed);
            failures++;
        } else if (won || c.player.hp <= 0) {
            completed++;
        }
    }

    printf("SELFTEST SUMMARY: rounds=%d completed=%d losses=%d failures=%d\n",
           rounds, completed, losses, failures);
    return failures == 0 ? 0 : 1;
}

int main(int argc, char** argv) {
    if (argc > 1 && strcmp(argv[1], "--self-test") == 0) {
        int rounds = 300;
        if (argc > 2) {
            rounds = atoi(argv[2]);
            if (rounds <= 0) rounds = 300;
        }
        return RunSelfTest(rounds);
    }

    must_init(al_init(), "allegro");

    // Resolve assets robustly when executable lives in build/.
    SetWorkingDirectoryForAssets();

    must_init(al_install_keyboard(), "keyboard");

    // 30 fps
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 30.0);
    must_init(timer, "timer");

    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    must_init(queue, "queue");

    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_timer_event_source(timer));

    unsigned char keyboard_keys[ALLEGRO_KEY_MAX];
    ClearKeyboardKeys(keyboard_keys);

    srand((unsigned int) time(NULL));

    ALLEGRO_EVENT event;

    Combat combat = {0};

    Renderer renderer = {0};
    FillRenderer(&renderer);
    renderer.combat = &combat;
    start_combat(&combat, &renderer);
    

    al_register_event_source(queue, al_get_display_event_source(renderer.display));

    al_start_timer(timer);

    bool done = false;
    bool redraw = false;
    while (!done) {
        al_wait_for_event(queue, &event);
        
        switch (event.type) {
            case ALLEGRO_EVENT_TIMER:
                update_combat(&combat, keyboard_keys, &renderer);
                redraw = true;

                if (keyboard_keys[ALLEGRO_KEY_Q]) {
                    done = true;
                }

                for (int i = 0; i < ALLEGRO_KEY_MAX; i++) {
                    keyboard_keys[i] &= ~GAME_KEY_SEEN;
                }
                break;

            case ALLEGRO_EVENT_KEY_DOWN:
                if (event.keyboard.keycode >= 0 && event.keyboard.keycode < ALLEGRO_KEY_MAX)
                    keyboard_keys[event.keyboard.keycode] = GAME_KEY_SEEN | GAME_KEY_DOWN;
                break;

            case ALLEGRO_EVENT_KEY_UP:
                if (event.keyboard.keycode >= 0 && event.keyboard.keycode < ALLEGRO_KEY_MAX)
                    keyboard_keys[event.keyboard.keycode] &= ~GAME_KEY_DOWN;
                break;

            case ALLEGRO_EVENT_DISPLAY_CLOSE:
                done = true;
                break;
        }
        
        if (redraw && al_is_event_queue_empty(queue)) {
            redraw = false;
            Render(&renderer);
        }
    }

    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    ClearRenderer(&renderer);

    return 0;
}

