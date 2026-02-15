#pragma once

#include <pebble.h>
#include "../defines.h"
#include "enums.h"

#define INNER_EXPL 0
#define OUTER_EXPL 1

extern TileType EXPLOSION_TILES[NUM_DIRS + 1][2];

extern Direction EXPLOSION_DIRECTIONS[EXPL_NUM_SQUARES];

extern bool EXPLOSION_EDGES[EXPL_NUM_SQUARES];
    