#pragma once

#include <pebble.h>

typedef enum {
    D_DOWN,
    D_LEFT,
    D_UP,
    D_RIGHT,
    D_NONE
} Direction;

typedef enum {
    T_NONE,
    T_WALL,
    T_ROCK,
    T_ROCK_BROKEN,
    T_ITEM_BOMB,
    T_ITEM_RANGE,
    T_ITEM_SHIELD,
    T_ITEM_KICK,
    T_ITEM_POWER,
    T_ITEM_BROKEN,
    T_BOMB,
    T_BOMB_3,
    T_BOMB_2,
    T_BOMB_1,
    T_EXPL_WEST_EDGE,
    T_EXPL_WEST,
    T_EXPL_CENTER,
    T_EXPL_EAST,
    T_EXPL_EAST_EDGE,
    T_EXPL_NORTH_EDGE,
    T_EXPL_NORTH,
    T_EXPL_SOUTH,
    T_EXPL_SOUTH_EDGE,
    T_SMALL_EXPL_WEST_EDGE,
    T_SMALL_EXPL_WEST,
    T_SMALL_EXPL_CENTER,
    T_SMALL_EXPL_EAST,
    T_SMALL_EXPL_EAST_EDGE,
    T_SMALL_EXPL_NORTH_EDGE,
    T_SMALL_EXPL_NORTH,
    T_SMALL_EXPL_SOUTH,
    T_SMALL_EXPL_SOUTH_EDGE,
} TileType;

typedef enum {
    TA_NONE,
    TA_BLOCK,
    TA_ITEM_BOMB,
    TA_ITEM_RANGE,
    TA_ITEM_SHIELD,
    TA_ITEM_KICK,
    TA_ITEM_POWER,
    TA_BOMB,
    TA_DEATH
} TileActions;

typedef enum {
    GT_VS_CPU,
    GT_QUICK_MATCH,
    GT_VS_FRIEND
} GameTypes;

typedef enum {
    GS_PLAYING,
    GS_GAME_OVER
} GameState;

typedef enum {
    P_STAND,
    P_WALK,
    P_DYING,
    P_DEAD
} PlayerState;

typedef enum {
    Q_NONE, // 0
    Q_UP, // 1
    Q_DOWN, // 2
    Q_BACK, // 3
    Q_SELECT, // 4
    Q_SELECT_DOUBLE, // 5
    Q_SELECT_HOLD, // 6
    Q_SELECT_PRESS, // 7
    Q_SELECT_RELEASE, // 8
    Q_UP_LONG, // 9
    Q_DOWN_LONG, // 10
} QueuedInput;

typedef enum {
    S_MENUS,
    S_PLAY
} AppState;

typedef enum {
    BS_COUNTDOWN,
    BS_DONE
} BombState;

typedef enum {
    ES_COUNTDOWN,
    ES_DONE
} ExplState;

typedef enum {
    RS_COUNTDOWN,
    RS_DONE
} RockState;

typedef enum {
    IS_COUNTDOWN,
    IS_DONE
} ItemState;

typedef enum {
    PA_PLACE_BOMB,
    PA_DIE,
    PA_KICK_BOMB
} PlayerAction;

typedef enum {
    PS_STAND_DOWN,
    PS_WALK_DOWN,
    PS_STAND_LEFT,
    PS_WALK_LEFT,
    PS_STAND_UP,
    PS_WALK_UP,
    PS_VICTORY,
    PS_DEFEAT,
    PS_DEATH
} PlayerSprite;

typedef enum {
    PT_HUMAN, // Me and you
    PT_COWARD, // Prioritize safety
    PT_HUNTER, // Prioritize humans
    PT_LAZY, // Takes longer to think
    PT_HOARDER, // Prioritizes items
    PT_DEMO, // Prioritizes rocks
    PT_FRIEND, // Prioritizes CPUs
    PT_SMART, // Prioritizes by path score: items, players
} PlayerTypes;

typedef enum {
    HT_PLAYER,
    HT_HUMAN,
    HT_CPU,
    HT_ITEMS,
    HT_ROCKS,
    HT_SAFETY
} HuntTypes;