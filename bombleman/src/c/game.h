#pragma once

#include <pebble.h>
#include "defines.h"
#include "structs/enums.h"
#include "player.h"
#include "pebble-gbc-graphics-advanced/pebble-gbc-graphics-advanced.h"
#include "bomb.h"
#include "explosion.h"
#include "rock.h"
#include "item.h"

typedef struct _game Game;
struct _game {
    Player *players[MAX_PLAYERS];
    Player *player_one;
    GBC_Graphics *graphics;
    Window *window;
    GameTypes game_type;
    // Settings *settings;
    int accel_cal_x, accel_cal_y;
    bool in_focus, paused;
    // Text layer for time remaining
    uint8_t map[SCREEN_BLOCK_SIZE];
    uint8_t player_spawns[MAX_PLAYERS * 2];
    int8_t scores[SCREEN_BLOCK_SIZE];
    Bomb *bombs[MAX_BOMBS_PER_GAME];
    Explosion *explosions[MAX_EXPLOSIONS_PER_GAME];
    Rock *rocks[MAX_ROCKS_PER_GAME];
    Item *items[MAX_ITEMS_PER_GAME];
    uint8_t explosion_frame;
    uint8_t num_players;
    uint8_t num_humans;
    int8_t cpu_paths[MAX_PLAYERS][PATH_MAX_LEN * 2];
    uint8_t cpu_path_lens[MAX_PLAYERS];
    int8_t cpu_path_scores[MAX_PLAYERS];
    uint8_t path_tick;
};

Game *Game_init(GBC_Graphics *graphics, Window *window, GameTypes game_type, uint8_t num_players, uint8_t num_humans);

void Game_destroy(Game *game);

void Game_render(Game *game);

void Game_start(Game *game);

// void Game_set_player_position(Game *game, char *username, uint16_t x, uint16_t y, Direction dir, bool animate);

int8_t Game_get_bomb_at_location(Game *game, uint8_t x, uint8_t y);

void Game_explode_bomb(Game *game, uint8_t bomb_idx);

bool Game_step(Game *game); // Returns true if game is still running

void Game_input_handler(Game *game, QueuedInput input);

void Game_up_handler(Game *game);

void Game_down_handler(Game *game);

void Game_select_handler(Game *game, QueuedInput input);

void Game_up_long_handler(Game *game);

void Game_down_long_handler(Game *game);

void Game_back_handler(Game *game);

void Game_tap_handler(Game *game);

void Game_focus_handler(Game *game, bool in_focus);