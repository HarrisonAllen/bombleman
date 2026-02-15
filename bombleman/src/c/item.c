#include "item.h"
#include "structs/enums.h"
#include "defines.h"
#include "utility.h"

Item *Item_initialize(uint8_t x, uint8_t y, uint8_t *map) {
    Item *item = NULL;
    item = malloc(sizeof(Item));
    if (item == NULL)
        return NULL;
    item->x = x;
    item->y = y;
    item->map = map;
    set_square(item->x, item->y, item->map, T_ITEM_BROKEN);
    item->state = IS_COUNTDOWN;
    item->time_left = ITEM_TICK_LIFETIME;
    return item;
}

void Item_destroy(Item *item) {
    if (item == NULL) return;
    if (item != NULL) {
        free(item);
    }
}

void Item_expire(Item *item) {
    item->state = IS_DONE;
}

void Item_step(Item *item) {
    switch (item->state) {
        case IS_COUNTDOWN:
            item->time_left--;
            if (item->time_left > ITEM_FADE_START) {
                set_square(item->x, item->y, item->map, T_ITEM_BROKEN);
            } else if (item->time_left > 0) {
                set_square(item->x, item->y, item->map, (item->time_left % 2) == 1 ? T_NONE : T_ITEM_BROKEN);
            } else {
                Item_expire(item);
            }
            break;
        case IS_DONE:
            break;
        default:
            break;
    }
}