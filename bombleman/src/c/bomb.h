#pragma once

#include <pebble.h>
#include "structs/enums.h"

typedef struct _bomb Bomb;
struct _bomb {
    uint8_t x;
    uint8_t y;
    uint8_t *map;
    uint8_t explosion_radius;
    uint8_t time_left;
    BombState state;
    uint8_t owner;
};

Bomb *Bomb_initialize(uint8_t owner, uint8_t x, uint8_t y, uint8_t expl_radius, uint8_t lifetime, uint8_t *map);

void Bomb_destroy(Bomb *bomb);

void Bomb_explode(Bomb *bomb);

void Bomb_move(Bomb *bomb, uint8_t x, uint8_t y);

void Bomb_step(Bomb *bomb);
