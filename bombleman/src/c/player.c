#include "player.h"
#include "utility.h"
#include "structs/tiles_to.h"
#include "palettes/palettes.h"

Player *Player_initialize(uint8_t number, PlayerTypes player_type, int16_t x, int16_t y, GBC_Graphics *graphics, uint8_t *map, void (*game_callback)(void *, Player *, PlayerAction), void *game) {
    Player *player = NULL;
    player = malloc(sizeof(Player));
    if (player == NULL)
        return NULL;
    player->number = number;
    player->player_type = player_type;
    player->graphics = graphics;
    player->map = map;
    player->x = x;
    player->y = y;
    player->dir = D_DOWN;
    player->needs_pos_broadcast = false;
    player->tile_offset = 0;
    player->queued_input = Q_NONE;
    player->bomb_count = 0;
    player->max_bombs = STARTING_BOMBS;
    player->explosion_radius = STARTING_EXPL_RADIUS;
    player->game_callback = game_callback;
    player->game = game;
    player->invincibility_frames = SPAWN_INVULN_FRAMES;
    player->state = P_STAND;
    player->walk_frame = 0;
    player->dying_frame = 0;
    player->select_pressed = false;
    player->logic_timer = CPU_START_TIMER_DELAY + rand() % (PT_LAZY ? CPU_SLOW_TIMER_DELAY : CPU_TIMER_DELAY);
    player->can_kick = false;

    uint8_t attrs = GBC_Graphics_attr_make(0, PLAYER_VRAM, false, false, true);
    GBC_Graphics_oam_set_sprite(player->graphics, player->number, player->x, player->y, 0, attrs, PLAYER_SPRITE_WIDTH - 1, PLAYER_SPRITE_HEIGHT - 1, 0, 0);
    GBC_Graphics_set_sprite_palette_array(player->graphics, player->number, PLAYER_PALETTES[player->number]);
    Player_show(player);
    Player_render(player);

    return player;
}

void Player_destroy(Player *player) {
    if (player == NULL) return;
    if (player != NULL) {
        free(player);
    }
}

bool Player_cpu_ready(Player *player) {
    return (
        player->player_type != PT_HUMAN
        && player->state != P_DYING
        && player->state != P_DEAD
        && player->logic_timer == 0 
        && (rand() % CPU_TIMER_WAIT_ODDS) != 0
    );
}

void Player_push_input(Player *player, QueuedInput input) {
    if (input == Q_SELECT_PRESS) {
        player->select_pressed = true;
    } else if (input == Q_SELECT_RELEASE) {
        player->select_pressed = false;
    } else {
        player->queued_input = input;
    }
}

void Player_place_bomb(Player *player) {
    player->game_callback(player->game, player, PA_PLACE_BOMB);
    player->logic_timer += player->player_type == PT_LAZY ? CPU_SLOW_TIMER_DELAY : CPU_TIMER_DELAY;
}

void Player_kick_bomb(Player *player) {
    player->game_callback(player->game, player, PA_KICK_BOMB);
    player->logic_timer += player->player_type == PT_LAZY ? CPU_SLOW_TIMER_DELAY : CPU_TIMER_DELAY;
}

void Player_pop_input(Player *player) {
    // if (player->settings->Tilt) {
    //     Direction old_dir = player->data.dir;
    //     Player_set_direction(player, player->tilt_direction);
    //     if (old_dir != player->data.dir) {
    //         player->needs_pos_broadcast = true;
    //     }
    // }
    if (player->queued_input == Q_UP) {
        Player_rotate_counterclockwise(player);
        player->needs_pos_broadcast = true;
        player->queued_input = Q_NONE;
    }
    if (player->queued_input == Q_DOWN) {
        Player_rotate_clockwise(player);
        player->needs_pos_broadcast = true;
        player->queued_input = Q_NONE;
    }
    if (player->queued_input == Q_BACK) {
        Player_place_bomb(player);  
        player->queued_input = Q_NONE;
    }
    
    bool can_walk_to = false;

    uint8_t next_x, next_y;
    get_square_in_direction(player->x, player->y, player->dir, &next_x, &next_y);
    can_walk_to = !is_square_blocking(next_x, next_y, player->map, player->invincibility_frames > 0);

    if (player->queued_input == Q_SELECT || player->select_pressed) {
        if (can_walk_to) {
            // Standing? Take a step
            player->state = P_WALK;
            player->walk_frame = 0;
            Player_set_target_pos(player);
            // if (player->number == 0) {
            //     broadcast_position(player->target_x, player->target_y, player->dir);
            // }
        } else {
            if (player->can_kick && is_square_bomb(next_x, next_y, player->map)) {
                Player_kick_bomb(player);
                player->select_pressed = false;
            }
            player->state = P_STAND;
            player->walk_frame = 0;
        }
    } else if (player->queued_input == Q_NONE) {
        if (player->state == P_WALK) {
            // Nothing pressed, in step, so stop    
            player->state = P_STAND;
            player->walk_frame = 0;
        }
    }
    player->queued_input = Q_NONE;
}

// void Player_stop_moving(Player *player) {
//     if (player->state == P_WALK) {
//         player->walk_state = W_STEP;
//     }
// }

void Player_hide(Player *player) {
    GBC_Graphics_oam_set_sprite_hidden(player->graphics, player->number, 1);
}

void Player_show(Player *player) {
    GBC_Graphics_oam_set_sprite_hidden(player->graphics, player->number, 0);
}

void Player_check_square(Player *player) {
    uint8_t *tile = player->map + to_square(player->x + (BLOCK_WIDTH / 2) * GBC_TILE_WIDTH) + to_square(player->y + (BLOCK_HEIGHT / 2) * GBC_TILE_HEIGHT) * SCREEN_BLOCK_WIDTH;
    TileActions tile_action = TILES_TO_ACTION[*tile];
    switch (tile_action) {
        case TA_ITEM_BOMB:
            *tile = T_NONE;
            player->max_bombs = GBC_MIN(player->max_bombs + 1, MAX_BOMBS_PER_PLAYER);
            break;
        case TA_ITEM_RANGE:
            *tile = T_NONE;
            player->explosion_radius += 1;
            break;
        case TA_ITEM_SHIELD:
            *tile = T_NONE;
            player->invincibility_frames = ITEM_SHIELD_INVULN_FRAMES;
            break;
        case TA_ITEM_KICK:
            *tile = T_NONE;
            player->can_kick = true;
            break;
        case TA_ITEM_POWER:
            *tile = T_NONE;
            break;
        case TA_DEATH:
            if (player->invincibility_frames == 0) {
                player->state = P_DYING;
                player->dying_frame = PLAYER_DEATH_FRAMES;
            }
            break;
        default:
            break;
    }
}

void Player_die(Player *player) {
    player->state = P_DEAD;
    player->game_callback(player->game, player, PA_DIE);
}

void Player_step(Player *player) {
    if (player->state == P_DEAD) return;
    if (player->invincibility_frames > 0) {
        // TODO: bug: for some reason, while invincible, the player palette changes? I think i fixed, keep an eye out
        player->invincibility_frames--;
        if (player->invincibility_frames % 2 == 0) {
            GBC_Graphics_set_sprite_palette_array(player->graphics, player->number, PLAYER_PALETTES[player->number]);
        } else {
            GBC_Graphics_set_sprite_palette_array(player->graphics, player->number, PLAYER_INV_PALETTES[player->number]);
        }
    }
    if (player->logic_timer > 0) {
        player->logic_timer--;
    } else {
        if ((rand() % CPU_TIMER_WAIT_ODDS) == 0) {
            player->logic_timer += rand() % (player->player_type == PT_LAZY ? CPU_SLOW_TIMER_WAIT_RANGE : CPU_TIMER_WAIT_RANGE);
        }
    }
    if (player->state == P_STAND) {
        Player_check_square(player);
        Player_pop_input(player);
    } else if (player->state == P_WALK) {
        Player_take_step(player, player->walk_frame + 1);
        player->walk_frame = (player->walk_frame + 1) % PLAYER_WALK_FRAME_COUNT;

        Player_check_square(player);
        if (player->walk_frame == 0) {
            Player_pop_input(player);
        }
    } else if (player->state == P_DYING) {
        player->dying_frame--;
        if (player->dying_frame == 0) {
            Player_die(player);
            Player_hide(player);
        }
    }
    // if (player->needs_pos_broadcast) {
    //     broadcast_position(player->target_x, player->target_y, player->data.dir);
    //     player->needs_pos_broadcast = false;
    // }
}

void Player_set_position(Player *player, int16_t x, int16_t y, Direction dir) {
    // x = clamp(PLAYER_MIN_X, x, PLAYER_MAX_X);
    // y = clamp(PLAYER_MIN_Y, y, PLAYER_MAX_Y);
    player->x = x;
    player->y = y;
    player->dir = dir;
}

void Player_move(Player *player, int16_t x, int16_t y, Direction dir) {
    Player_set_position(player, player->x + x, player->y + y, dir);
}

void Player_set_move(Player *player, int16_t *x, int16_t *y, int16_t dx, int16_t dy) {
    *x = *x + dx;
    *y = *y + dy;
}

void Player_set_sprites(Player *player) {
    uint8_t offset = 0;
    bool x_flip = false;

    if (player->state != P_DYING && player->state != P_DEAD) { // Standing/walking
        offset = PS_STAND_DOWN;
        x_flip = false;
        switch (player->dir) {
            case D_DOWN:
                offset += 0;
                x_flip = player->walk_frame == 3;
                break;
            case D_LEFT:
                offset += 2;
                break;
            case D_UP:
                offset += 4;
                x_flip = player->walk_frame == 3;
                break;
            case D_RIGHT:
                offset += 2;
                x_flip = true;
                break;
            default:
                break;
        }
        offset += (player->walk_frame % 2);
    } else if (player->state == P_DYING) {
        offset = PS_DEATH;
        x_flip = false;
        if (player->dying_frame < PLAYER_FADE_START && (player->dying_frame % 2) == 1) {
            Player_hide(player);
        } else {
            Player_show(player);
        }
    } else if (player->state == P_DEAD) {
        Player_hide(player);
    }
    GBC_Graphics_oam_set_sprite_tile(player->graphics, player->number, offset * PLAYER_SPRITE_SIZE);
    GBC_Graphics_oam_set_sprite_x_flip(player->graphics, player->number, x_flip);
}

void Player_render(Player *player) {
    // Player_show(player);
    Player_set_sprites(player);
    GBC_Graphics_oam_set_sprite_pos(player->graphics, 
        player->number, 
        GBC_SPRITE_OFFSET_X - SCREEN_OFFSET.x + player->x + PLAYER_SPRITE_OFFSET_X,
        GBC_SPRITE_OFFSET_Y - SCREEN_OFFSET.y + player->y + PLAYER_SPRITE_OFFSET_Y);
}

void Player_set_direction(Player *player, Direction direction) {
    if (direction != D_NONE) {
        player->dir = direction;
    }
}

void Player_rotate_clockwise(Player *player) {
    Player_set_direction(player, (player->dir + 1 + D_NONE) % D_NONE);
}

void Player_rotate_counterclockwise(Player *player) {
    Player_set_direction(player, (player->dir - 1 + D_NONE) % D_NONE); // The + D_NONE is to avoid negative mod
}

void Player_set_tilt_direction(Player *player, Direction tilt_direction) {
    player->tilt_direction = tilt_direction;
}

void Player_take_step(Player *player, uint8_t walk_frame) {
    Player_set_position(player, frame_lerp(walk_frame, PLAYER_WALK_FRAME_COUNT, player->start_x, player->target_x),
                                frame_lerp(walk_frame, PLAYER_WALK_FRAME_COUNT, player->start_y, player->target_y),
                                player->dir);
}

void Player_walk_to_location(Player *player, int16_t x, int16_t y, Direction dir) {
    player->start_x = player->x;
    player->start_y = player->y;

    player->target_x = x;
    player->target_y = y;
    player->walk_frame = 0;
    if (!(player->start_x == player->target_x && player->start_y == player->target_y)) {
        player->state = P_WALK;
        player->dir = pos_delta_to_dir(player->target_x - player->start_x, player->target_y - player->start_y);
        player->logic_timer += player->player_type == PT_LAZY ? CPU_SLOW_TIMER_DELAY : CPU_TIMER_DELAY;
    } else {
        player->dir = dir;
    }
}

void Player_set_target_pos(Player *player) {
    int16_t target_x = player->x;
    int16_t target_y = player->y;
    switch (player->dir) {
        case D_UP:
            Player_set_move(player, &target_x, &target_y, 0, -PLAYER_STEP_SIZE);
            break;
        case D_DOWN:
            Player_set_move(player, &target_x, &target_y, 0, PLAYER_STEP_SIZE);
            break;
        case D_LEFT:
            Player_set_move(player, &target_x, &target_y, -PLAYER_STEP_SIZE, 0);
            break;
        case D_RIGHT:
            Player_set_move(player, &target_x, &target_y, PLAYER_STEP_SIZE, 0);
            break;
        default:
            break;
    }
    Player_walk_to_location(player, target_x, target_y, player->dir);
}
