#include "explosion.h"
#include "structs/enums.h"
#include "defines.h"
#include "utility.h"
#include "structs/explosion_tiles.h"

Explosion *Explosion_initialize(uint8_t x, uint8_t y, uint8_t *bounds, uint8_t expl_radius, uint8_t lifetime, uint8_t *map) {
    Explosion *explosion = NULL;
    explosion = malloc(sizeof(Explosion));
    if (explosion == NULL)
        return NULL;
    explosion->x = x;
    explosion->y = y;
    memcpy(explosion->bounds, bounds, NUM_DIRS * 2);
    explosion->time_left = lifetime;
    explosion->explosion_radius = expl_radius;
    explosion->map = map;
    explosion->state = ES_COUNTDOWN;
    // Explosion_render(explosion);
    return explosion;
}

void Explosion_destroy(Explosion *explosion) {
    if (explosion == NULL) return;
    if (explosion != NULL) {
        free(explosion);
    }
}

void Explosion_render(Explosion *explosion, uint8_t explosion_frame) {
    uint8_t offset = explosion_frame * EXPL_NUM_SQUARES;
    // Set center
    uint8_t x = explosion->x;
    uint8_t y = explosion->y;
    for (uint8_t i = 0; i < NUM_DIRS; i++) {
        x = explosion->bounds[i*2];
        y = explosion->bounds[i*2 + 1];
        while (x != explosion->x || y != explosion->y) {
            bool is_this_edge = !(abs((int8_t)x - explosion->x) < explosion->explosion_radius && abs((int8_t)y - explosion->y) < explosion->explosion_radius);
            Direction this_dir = i;
            TileType other_tile = explosion->map[x + y * SCREEN_BLOCK_WIDTH];
            if (other_tile >= T_EXPL_WEST_EDGE) {
                Direction other_dir = square_explosion_direction(x, y, explosion->map);
                bool is_other_edge = is_square_explosion_edge(x, y, explosion->map);
                if (this_dir == other_dir || ((this_dir + 2) % NUM_DIRS) == other_dir) {
                    if (is_this_edge && is_other_edge) {
                        set_square(x, y, explosion->map, EXPLOSION_TILES[i][1] + offset);
                    } else {
                        set_square(x, y, explosion->map, EXPLOSION_TILES[i][0] + offset);
                    }
                } else {
                    set_square(x, y, explosion->map, T_EXPL_CENTER + offset);
                }
            } else {
                if (is_this_edge) {
                    set_square(x, y, explosion->map, EXPLOSION_TILES[i][1] + offset);
                } else {
                    set_square(x, y, explosion->map, EXPLOSION_TILES[i][0] + offset);
                }
            }

            x -= X_Y_DIRS[i][0];
            y -= X_Y_DIRS[i][1];
        }
    }
    set_square(explosion->x, explosion->y, explosion->map, T_EXPL_CENTER + offset);
}

void Explosion_expire(Explosion *explosion) {
    uint8_t x = explosion->x;
    uint8_t y = explosion->y;
    for (uint8_t i = 0; i < NUM_DIRS; i++) {
        x = explosion->bounds[i*2];
        y = explosion->bounds[i*2 + 1];
        while (x != explosion->x || y != explosion->y) {
            set_square(x, y, explosion->map, T_NONE);
            x -= X_Y_DIRS[i][0];
            y -= X_Y_DIRS[i][1];
        }
    }
    // set_square(explosion->x, explosion->y, explosion->map, T_NONE);
    explosion->state = ES_DONE;
}

void Explosion_step(Explosion *explosion) {
    switch (explosion->state) {
        case ES_COUNTDOWN:
            explosion->time_left--;
            if (explosion->time_left > 0) {
                // Explosion_render(explosion);
            } else {
                Explosion_expire(explosion);
            }
            break;
        case ES_DONE:
            break;
        default:
            break;
    }
}