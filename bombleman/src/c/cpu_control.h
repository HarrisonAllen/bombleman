#pragma once
#include <pebble.h>

bool path_contains_point(int8_t x, int8_t y, int8_t *path, uint8_t path_len);

int8_t find_path(int8_t x, int8_t y, int8_t end_x, int8_t end_y, uint8_t *map, int8_t *path, uint8_t *path_len);    