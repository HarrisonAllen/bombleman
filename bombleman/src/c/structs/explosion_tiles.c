#include "explosion_tiles.h"

TileType EXPLOSION_TILES[NUM_DIRS + 1][2] = {
    {T_EXPL_SOUTH, T_EXPL_SOUTH_EDGE},
    {T_EXPL_WEST, T_EXPL_WEST_EDGE},
    {T_EXPL_NORTH, T_EXPL_NORTH_EDGE},
    {T_EXPL_EAST, T_EXPL_EAST_EDGE},
    {T_EXPL_CENTER, T_EXPL_CENTER},
};

Direction EXPLOSION_DIRECTIONS[EXPL_NUM_SQUARES] = {
    D_LEFT,
    D_LEFT,
    D_NONE,
    D_RIGHT,
    D_RIGHT,
    D_UP,
    D_UP,
    D_DOWN,
    D_DOWN
};

bool EXPLOSION_EDGES[EXPL_NUM_SQUARES] = {
    true,
    false,
    false,
    false,
    true,
    true,
    false,
    false,
    true
};