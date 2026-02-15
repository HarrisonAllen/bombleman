#pragma once
#include <pebble.h>
#include "structs/enums.h"

void load_byte_range_from_resource(uint32_t resource, uint32_t index, uint32_t num_bytes, uint8_t *buffer);

uint8_t load_map(uint8_t *map_buffer, uint32_t resource, uint8_t map_width, uint8_t map_height, uint8_t x_inset, uint8_t y_inset, uint8_t *player_spawns);

uint8_t to_square(int16_t a);

int16_t from_square(uint8_t a);    

void get_square(int16_t x, int16_t y, uint8_t *x_out, uint8_t *y_out);

void get_square_in_direction(int16_t x, int16_t y, Direction dir, uint8_t *x_out, uint8_t *y_out);

bool is_square_blocking(uint8_t x, uint8_t y, uint8_t *map, bool invulnerable);

bool is_square_bomb(uint8_t x, uint8_t y, uint8_t *map);

bool is_square_empty(uint8_t x, uint8_t y, uint8_t *map);

bool is_square_explodable(uint8_t x, uint8_t y, uint8_t *map);

bool is_square_destroyable(uint8_t x, uint8_t y, uint8_t *map);

bool is_square_safe(uint8_t x, uint8_t y, int8_t *scores, bool invulnerable);

bool is_next_square_safer(uint8_t cur_x, uint8_t cur_y, uint8_t next_x, uint8_t next_y, int8_t* scores);

bool is_square_explosion_edge(uint8_t x, uint8_t y, uint8_t *map);

Direction square_explosion_direction(uint8_t x, uint8_t y, uint8_t *map);

void set_square(uint8_t x, uint8_t y, uint8_t *map, TileType tile);

int16_t clamp(int16_t low, int16_t value, int16_t high);

int16_t frame_lerp(uint8_t frame, uint8_t end_frame, int16_t value_start, int16_t value_end);

Direction pos_delta_to_dir(int16_t dx, int16_t dy);

int16_t manhattan_distance(int16_t x1, int16_t y1, int16_t x2, int16_t y2);

Direction x_y_delta_to_dir(int8_t x_delta, int8_t y_delta);