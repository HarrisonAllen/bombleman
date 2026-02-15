#pragma once

#include <pebble.h>
#include "structs/enums.h"

typedef struct _rock Rock;
struct _rock {
    uint8_t x;
    uint8_t y;
    uint8_t *map;
    uint8_t time_left;
    RockState state;
};

Rock *Rock_initialize(uint8_t x, uint8_t y, uint8_t *map);

void Rock_destroy(Rock *rock);

void Rock_expire(Rock *rock);

void Rock_step(Rock *rock);
