#include "bomb.h"
#include "structs/enums.h"
#include "defines.h"
#include "utility.h"

Bomb *Bomb_initialize(uint8_t owner, uint8_t x, uint8_t y, uint8_t expl_radius, uint8_t lifetime, uint8_t *map) {
    Bomb *bomb = NULL;
    bomb = malloc(sizeof(Bomb));
    if (bomb == NULL)
        return NULL;
    bomb->owner = owner;
    bomb->x = x;
    bomb->y = y;
    bomb->time_left = lifetime;
    bomb->explosion_radius = expl_radius;
    bomb->map = map;
    set_square(bomb->x, bomb->y, bomb->map, T_BOMB);
    bomb->state = BS_COUNTDOWN;
    return bomb;
}

void Bomb_destroy(Bomb *bomb) {
    if (bomb == NULL) return;
    if (bomb != NULL) {
        free(bomb);
    }
}

void Bomb_explode(Bomb *bomb) {
    // set_square(bomb->x, bomb->y, bomb->map, T_NONE);
    bomb->state = BS_DONE;
}

void Bomb_move(Bomb *bomb, uint8_t x, uint8_t y) {
    set_square(bomb->x, bomb->y, bomb->map, T_NONE);
    bomb->x = x;
    bomb->y = y;
    set_square(bomb->x, bomb->y, bomb->map, T_BOMB);
}

void Bomb_step(Bomb *bomb) {
    switch (bomb->state) {
        case BS_COUNTDOWN:
            bomb->time_left--;
            if (bomb->time_left > BOMB_COUNTDOWN_START) {
                set_square(bomb->x, bomb->y, bomb->map, T_BOMB);
            } else if (bomb->time_left > 0) {
                uint8_t frame = BOMB_COUNT_FRAMES - (bomb->time_left - 1)  / FRAMERATE;
                set_square(bomb->x, bomb->y, bomb->map, T_BOMB + frame);
            } else {
                Bomb_explode(bomb);
            }
            break;
        case BS_DONE:
            break;
        default:
            break;
    }
}