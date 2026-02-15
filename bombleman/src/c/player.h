#pragma once

#include <pebble.h>
#include "defines.h"
#include "structs/enums.h"
#include "pebble-gbc-graphics-advanced/pebble-gbc-graphics-advanced.h"

typedef struct _player Player;
struct _player {
    GBC_Graphics *graphics;
    Direction tilt_direction;
    uint8_t number;
    uint8_t palette[GBC_PALETTE_NUM_BYTES];
    PlayerState state;
    uint8_t walk_frame;
    uint8_t dying_frame;
    uint8_t logic_timer;
    uint8_t tile_offset;
    QueuedInput queued_input;
    bool select_pressed;
    int16_t target_x, target_y;
    int16_t start_x, start_y;
    bool needs_pos_broadcast;
    int16_t x;
    int16_t y;
    Direction dir;
    uint8_t *map;
    uint8_t max_bombs;
    int8_t bomb_count;
    uint8_t explosion_radius;
    void (*game_callback)(void *game, Player *, PlayerAction);
    void *game;
    uint8_t invincibility_frames;
    PlayerTypes player_type;
    bool can_kick;
};

Player *Player_initialize(uint8_t number, PlayerTypes player_type, int16_t x, int16_t y, GBC_Graphics *graphics, uint8_t *map, void (*game_callback)(void *, Player *, PlayerAction), void *game);

void Player_destroy(Player *player);

bool Player_cpu_ready(Player *player);

void Player_push_input(Player *player, QueuedInput input);

void Player_place_bomb(Player *player);

void Player_kick_bomb(Player *player);

void Player_pop_input(Player *player);

void Player_die(Player *player);

void Player_step(Player *player);

void Player_set_position(Player *player, int16_t x, int16_t y, Direction dir);

void Player_move(Player *player, int16_t x, int16_t y, Direction dir);

void Player_set_sprites(Player *player);

// void Player_set_palette(Player *player);

void Player_show(Player *player);

void Player_hide(Player *player);

void Player_render(Player *player);

void Player_set_direction(Player *player, Direction direction);

void Player_rotate_clockwise(Player *player);

void Player_rotate_counterclockwise(Player *player);

void Player_set_tilt_direction(Player *player, Direction tilt_direction);

void Player_take_step(Player *player, uint8_t walk_frame);

void Player_walk_to_location(Player *player, int16_t x, int16_t y, Direction dir);

void Player_set_target_pos(Player *player);
