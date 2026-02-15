#pragma once

#include <pebble.h>
#include "structs/enums.h"
#include "defines.h"

typedef struct _explosion Explosion;
struct _explosion {
    uint8_t x;
    uint8_t y;
    uint8_t *map;
    uint8_t bounds[NUM_DIRS * 2]; // west xy, north xy, east xy, south xy
    uint8_t explosion_radius;
    uint8_t time_left;
    ExplState state;
};

Explosion *Explosion_initialize(uint8_t x, uint8_t y, uint8_t *bounds, uint8_t expl_radius, uint8_t lifetime, uint8_t *map);


void Explosion_destroy(Explosion *explosion);

void Explosion_render(Explosion *explosion, uint8_t explosion_frame);

void Explosion_expire(Explosion *explosion);

void Explosion_step(Explosion *explosion);
