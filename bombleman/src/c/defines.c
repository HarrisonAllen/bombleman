#include "defines.h"
#include "structs/enums.h"

const int16_t PLAYER_SPAWNS[MAX_PLAYERS][2] = {
    {16, 16},
    {144, 112},
    {16, 112},
    {144, 16},
};

// Directions - x, y
const int8_t X_Y_DIRS[NUM_DIRS+1][2] = {
    {0, 1},   // Down
    {-1, 0}, // Left
    {0, -1}, // Up
    {1, 0},  // Right
    {0, 0}, // None
};

const int8_t X_Y_MULTS[NUM_DIRS][2] = {
    {0, 1},   // Down
    {1, 0}, // Left
    {0, 1}, // Up
    {1, 0},  // Right
};

const uint8_t PATH_ORDERS[NUM_DIRS][NUM_DIRS] = {
    {D_DOWN, D_LEFT, D_RIGHT, D_UP}, // Down
    {D_LEFT, D_UP, D_DOWN, D_RIGHT}, // Left
    {D_UP, D_RIGHT, D_LEFT, D_DOWN}, // Up
    {D_RIGHT, D_DOWN, D_UP, D_LEFT}, // Right
};