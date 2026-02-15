#include "cpu_control.h"
#include "structs/enums.h"
#include "defines.h"

bool path_contains_point(int8_t x, int8_t y, int8_t *path, uint8_t path_len) {
    int8_t path_x, path_y;
    for (int8_t i = 0; i < path_len; i++) {
        path_x = path[i*2];
        path_y = path[i*2 + 1];
        if (x == path_x && y == path_y) {
            return true;
        }
        // Path is populated with -1s, so they'll indicate end of the path
        if (x == -1 || y == -1) { 
            return false;
        }
    }
    return false;
}

int8_t find_path(int8_t x, int8_t y, int8_t end_x, int8_t end_y, uint8_t *map, int8_t *path, uint8_t *path_len) {
    if (*path_len >= PATH_MAX_LEN) {
        return SCORE_PATH_FAIL;
    }
    if (path_contains_point(x, y, path, *path_len)) {
        return SCORE_PATH_FAIL;
    }
    if (x == end_x && y == end_y) {
        path[*path_len * 2] = x;
        path[*path_len * 2 + 1] = y;
        *path_len = *path_len + 1;
        return SCORE_PATH_END;
    }
    TileType block = map[x + y * SCREEN_BLOCK_WIDTH];
    if (block == T_WALL) {
        return SCORE_PATH_WALL;
    }
    // TODO: if bomb, and not invincible, it's a wall
    int8_t block_score = block == T_ROCK ? SCORE_PATH_ROCK : SCORE_PATH_NONE;
    path[*path_len * 2] = x;
    path[*path_len * 2 + 1] = y;
    
    // At this point one of x_dist, y_dist is guaranteed to be != 0
    int8_t x_dist = end_x - x;
    int8_t y_dist = end_y - y;

    // Figure out what order of directions to search
    Direction path_order[NUM_DIRS];
    if (abs(x_dist) < abs(y_dist)) { // Y is farther, go vertical then horizontal
        if (y_dist > 0) {
            path_order[0] = D_DOWN;
            path_order[3] = D_UP;
        } else {
            path_order[0] = D_UP;
            path_order[3] = D_DOWN;
        }
        if (x_dist < 0) {
            path_order[1] = D_LEFT;
            path_order[2] = D_RIGHT;
        } else {
            path_order[1] = D_RIGHT;
            path_order[2] = D_LEFT;
        }
    } else { // X is farther, go horizontal then vertical
        if (x_dist > 0) {
            path_order[0] = D_RIGHT;
            path_order[3] = D_LEFT;
        } else {
            path_order[0] = D_LEFT;
            path_order[3] = D_RIGHT;
        }
        if (y_dist > 0) {
            path_order[1] = D_DOWN;
            path_order[2] = D_UP;
        } else {
            path_order[1] = D_UP;
            path_order[2] = D_DOWN;
        }
    }

    // Now search these paths, take the first one
    int8_t path_score = -1;
    uint8_t temp_path_len = *path_len + 1;
    for (uint8_t i = 0; i < NUM_DIRS; i++) {
        path_score = find_path(x + X_Y_DIRS[path_order[i]][0], y + X_Y_DIRS[path_order[i]][1], end_x, end_y, map, path, &temp_path_len);
        if (path_score >= 0) { // We're done! Path found!
            *path_len = temp_path_len;
            return path_score + block_score;
        }
    }

    // Failed to find a path!
    return SCORE_PATH_FAIL;
}