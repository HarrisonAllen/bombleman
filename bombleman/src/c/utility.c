#include "utility.h"
#include "defines.h"
#include "structs/tiles_to.h"
#include "structs/explosion_tiles.h"

void load_byte_range_from_resource(uint32_t resource, uint32_t index, uint32_t num_bytes, uint8_t *buffer) {
    ResHandle handle = resource_get_handle(resource);
    resource_load_byte_range(handle, index, buffer, num_bytes);
}

uint8_t load_map(uint8_t *map_buffer, uint32_t resource, uint8_t map_width, uint8_t map_height, uint8_t x_inset, uint8_t y_inset, uint8_t *player_spawns) {
    uint8_t tile, posn;
    uint8_t num_spawns = 0;

    // Load map into buffer (to account for malformed maps)
    // Width + 1 because of newlines
    uint8_t full_map_buffer[(map_width + 1) * map_height];
    memset(full_map_buffer, T_NONE, (map_width + 1) * map_height);
    ResHandle handle = resource_get_handle(resource);
    size_t res_size = resource_size(handle);
    resource_load(handle, full_map_buffer, res_size);

    for (uint8_t y = 0; y < map_height; y++) {
        for (uint8_t x = 0; x < map_width; x++) {
            // index + map_width * 1 because there's a new line
            tile = full_map_buffer[x + y * (map_width + 1)];
            posn = (x + x_inset) + (y + y_inset) * (map_width + 2); // + 2 b/c walls are cut off
            if (tile >= '1' && tile < ('1' + MAX_PLAYERS)) { // Player spawn
                uint8_t player = tile - '1';
                map_buffer[posn] = T_NONE;
                player_spawns[player * 2] = x + x_inset;
                player_spawns[player * 2 + 1] = y + y_inset;
                num_spawns++;
            } else if (tile == '_') { // Blank tile
                map_buffer[posn] = T_NONE;
            } else if (tile >= 'a' && tile <= ('a' + MAP_EDITOR_NUM_TILES)) {
                map_buffer[posn] = TILES_FOR_MAP[tile - 'a'];
            } else {
                map_buffer[posn] = T_NONE;
            }
        }
    }
    return num_spawns;
}


uint8_t to_square(int16_t a) {
    return a / SQUARE_SIZE;
}

int16_t from_square(uint8_t a) {
    return a * SQUARE_SIZE;
}

void get_square(int16_t x, int16_t y, uint8_t *x_out, uint8_t *y_out) {
    *x_out = to_square(x);
    *y_out = to_square(y);
}

void get_square_in_direction(int16_t x, int16_t y, Direction dir, uint8_t *x_out, uint8_t *y_out) {
    *x_out = to_square(x);
    *y_out = to_square(y);
    switch (dir) {
        case D_UP:
            *y_out = *y_out - 1;
            break;
        case D_DOWN:
            *y_out = *y_out + 1;
            break;
        case D_LEFT:
            *x_out = *x_out - 1;
            break;
        case D_RIGHT:
            *x_out = *x_out + 1;
            break;
        default:
            break;
    }
}

bool is_square_blocking(uint8_t x, uint8_t y, uint8_t *map, bool invulnerable) {
    TileActions tile_action = TILES_TO_ACTION[map[x + y * SCREEN_BLOCK_WIDTH]];
    return tile_action == TA_BLOCK || (tile_action == TA_BOMB && !invulnerable);
}

bool is_square_bomb(uint8_t x, uint8_t y, uint8_t *map) {
    TileActions tile_action = TILES_TO_ACTION[map[x + y * SCREEN_BLOCK_WIDTH]];
    return tile_action == TA_BOMB;
}

bool is_square_empty(uint8_t x, uint8_t y, uint8_t *map) {
    return map[x + y * SCREEN_BLOCK_WIDTH] == T_NONE;
}

bool is_square_explodable(uint8_t x, uint8_t y, uint8_t *map) {
    TileType tile_action = map[x + y * SCREEN_BLOCK_WIDTH];
    return tile_action != T_WALL
    && tile_action != T_ROCK
    && tile_action != T_ROCK_BROKEN
    && tile_action != T_ITEM_BOMB
    && tile_action != T_ITEM_RANGE
    && tile_action != T_ITEM_SHIELD
    && tile_action != T_ITEM_KICK
    && tile_action != T_ITEM_POWER
    && tile_action != T_ITEM_BROKEN;
}

bool is_square_destroyable(uint8_t x, uint8_t y, uint8_t *map) {
    TileType tile_type = map[x + y * SCREEN_BLOCK_WIDTH];
    return tile_type == T_ROCK
    || tile_type == T_ITEM_BOMB
    || tile_type == T_ITEM_RANGE
    || tile_type == T_ITEM_SHIELD
    || tile_type == T_ITEM_KICK
    || tile_type == T_ITEM_POWER
    || tile_type == T_BOMB
    || tile_type == T_BOMB_3
    || tile_type == T_BOMB_2
    || tile_type == T_BOMB_1;
}
bool is_square_safe(uint8_t x, uint8_t y, int8_t *scores, bool invulnerable) {
    int8_t score = scores[x + y * SCREEN_BLOCK_WIDTH];
    return (
        score >= SCORE_SAFE_THRESHOLD 
        || (invulnerable && score >= SCORE_BLOCKING_THRESHOLD)
    );
}

bool is_next_square_safer(uint8_t cur_x, uint8_t cur_y, uint8_t next_x, uint8_t next_y, int8_t* scores) {
    return scores[cur_x + cur_y * SCREEN_BLOCK_WIDTH] < scores[next_x + next_y * SCREEN_BLOCK_WIDTH];
}

bool is_square_explosion_edge(uint8_t x, uint8_t y, uint8_t *map) {
    TileType tile_type = map[x + y * SCREEN_BLOCK_WIDTH];
    if (tile_type < T_EXPL_WEST_EDGE) return false;
    uint8_t expl_tile = (tile_type - T_EXPL_WEST_EDGE) % EXPL_NUM_SQUARES;
    return EXPLOSION_EDGES[expl_tile];
}

Direction square_explosion_direction(uint8_t x, uint8_t y, uint8_t *map) {
    TileType tile_type = map[x + y * SCREEN_BLOCK_WIDTH];
    if (tile_type < T_EXPL_WEST_EDGE) return false;
    uint8_t expl_tile = (tile_type - T_EXPL_WEST_EDGE) % EXPL_NUM_SQUARES;
    return EXPLOSION_DIRECTIONS[expl_tile];
}

void set_square(uint8_t x, uint8_t y, uint8_t *map, TileType tile) {
    map[x + y * SCREEN_BLOCK_WIDTH] = tile;
}

int16_t clamp(int16_t low, int16_t value, int16_t high) {
    if (value < low) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return value;
}

int16_t frame_lerp(uint8_t frame, uint8_t end_frame, int16_t value_start, int16_t value_end) {
    float percent = frame / (float)end_frame;
    int16_t value = value_start + percent * (value_end - value_start);
    return value;
}

Direction pos_delta_to_dir(int16_t dx, int16_t dy) {
    if (dx < 0) {
        return D_LEFT;
    }
    if (dx > 0) {
        return D_RIGHT;
    }
    if (dy < 0) {
        return D_UP;
    }
    return D_DOWN;
}

int16_t manhattan_distance(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}

Direction x_y_delta_to_dir(int8_t x_delta, int8_t y_delta) {
    if (abs(x_delta) > abs(y_delta)) {
        if (x_delta > 0) {
            return D_RIGHT;
        } else {
            return D_LEFT;
        }
    } else {
        if (y_delta < 0) {
            return D_UP;
        } else {
            return D_DOWN;
        }
    }
}