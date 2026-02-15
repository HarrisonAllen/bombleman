#include "rock.h"
#include "structs/enums.h"
#include "defines.h"
#include "utility.h"

Rock *Rock_initialize(uint8_t x, uint8_t y, uint8_t *map) {
    Rock *rock = NULL;
    rock = malloc(sizeof(Rock));
    if (rock == NULL)
        return NULL;
    rock->x = x;
    rock->y = y;
    rock->map = map;
    set_square(rock->x, rock->y, rock->map, T_ROCK_BROKEN);
    rock->state = RS_COUNTDOWN;
    rock->time_left = ROCK_TICK_LIFETIME;
    return rock;
}

void Rock_destroy(Rock *rock) {
    if (rock == NULL) return;
    if (rock != NULL) {
        free(rock);
    }
}

void Rock_expire(Rock *rock) {
    rock->state = RS_DONE;
}

void Rock_step(Rock *rock) {
    switch (rock->state) {
        case RS_COUNTDOWN:
            rock->time_left--;
            if (rock->time_left > ROCK_FADE_START) {
                set_square(rock->x, rock->y, rock->map, T_ROCK_BROKEN);
            } else if (rock->time_left > 0) {
                set_square(rock->x, rock->y, rock->map, (rock->time_left % 2) == 1 ? T_NONE : T_ROCK_BROKEN);
            } else {
                Rock_expire(rock);
            }
            break;
        case RS_DONE:
            break;
        default:
            break;
    }
}