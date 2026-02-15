#pragma once

#include <pebble.h>
#include "structs/enums.h"

typedef struct _item Item;
struct _item {
    uint8_t x;
    uint8_t y;
    uint8_t *map;
    uint8_t time_left;
    ItemState state;
};

Item *Item_initialize(uint8_t x, uint8_t y, uint8_t *map);

void Item_destroy(Item *item);

void Item_expire(Item *item);

void Item_step(Item *item);
