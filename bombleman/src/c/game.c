#include "game.h"
#include "utility.h"
#include "palettes/palettes.h"
#include "structs/tiles_to.h"
#include "cpu_control.h"

static int8_t map_choice = -1;

Game *Game_init(GBC_Graphics *graphics, Window *window, GameTypes game_type, uint8_t num_players, uint8_t num_humans) {
    Game *game = NULL;
    game = malloc(sizeof(Game));
    if (game == NULL)
        return NULL;
    game->graphics = graphics;
    game->window = window;
    game->game_type = game_type;
    game->num_players = num_players;
    game->num_humans = num_humans;
    game->path_tick = 0;
    memset(game->map, T_WALL, SCREEN_BLOCK_SIZE);
    memset(game->player_spawns, 1, sizeof(game->player_spawns));
    if (map_choice == -1)
    {
        map_choice = rand() % NUM_MAPS;
    }
    else
    {
        map_choice = (map_choice + 1) % NUM_MAPS;
    }
    load_map(game->map, START_MAP + map_choice, SCREEN_BLOCK_WIDTH-2, SCREEN_BLOCK_HEIGHT-2, 1, 1, game->player_spawns);
    memset(game->scores, SCORE_NULL, sizeof(game->scores));
    return game;
}

void Game_destroy(Game *game) {
    if (game == NULL) return;
    // Destroy players
    for (uint8_t i = 0; i < MAX_PLAYERS; i++) {
        if (game->players[i] != NULL) {
            Player_destroy(game->players[i]);
        }
    }
    // Destroy bombs
    for (uint8_t i = 0; i < MAX_BOMBS_PER_GAME; i++) {
        if (game->bombs[i] != NULL) {
            Bomb_destroy(game->bombs[i]);
        }
    }
    // Destroy explosions
    for (uint8_t i = 0; i < MAX_EXPLOSIONS_PER_GAME; i++) {
        if (game->explosions[i] != NULL) {
            Explosion_destroy(game->explosions[i]);
        }
    }
    // Destroy rocks
    for (uint8_t i = 0; i < MAX_ROCKS_PER_GAME; i++) {
        if (game->rocks[i] != NULL) {
            Rock_destroy(game->rocks[i]);
        }
    }
    // Destroy items
    for (uint8_t i = 0; i < MAX_ITEMS_PER_GAME; i++) {
        if (game->items[i] != NULL) {
            Item_destroy(game->items[i]);
        }
    }
    // Destroy game
    if (game != NULL) {
        free(game);
    }
}

void Game_render(Game *game) {
    uint8_t block, tile, attrs;
    for (uint8_t y = 0; y < SCREEN_TILE_HEIGHT; y++) {
        for (uint8_t x = 0; x < SCREEN_TILE_WIDTH; x++) {
            block = game->map[x/BLOCK_WIDTH + (y/BLOCK_HEIGHT) * SCREEN_BLOCK_WIDTH];
            tile = block * BLOCK_SIZE + x % 2 + (y % 2) * BLOCK_WIDTH;
            attrs = GBC_Graphics_attr_make(0, BACKGROUND_VRAM, false, false, false);
            GBC_Graphics_bg_set_tile_and_attrs(game->graphics, 0, x, y, tile, attrs);
        }
    }
    GBC_Graphics_render(game->graphics);
}

void Game_handle_player_action(void *game_pointer, Player *player, PlayerAction action) {
    Game *game = (Game *)game_pointer;
    uint8_t player_next_x, player_next_y;
    uint8_t bomb_next_x, bomb_next_y;
    int8_t bomb;
    switch (action) {
        case PA_PLACE_BOMB:
            if (!is_square_empty(to_square(player->x), to_square(player->y), player->map)) {
                return;
            }
            if (player->bomb_count < player->max_bombs) {
                for (uint8_t i = 0; i < MAX_BOMBS_PER_GAME; i++) {
                    if (game->bombs[i] == NULL) {
                        game->bombs[i] = Bomb_initialize(player->number, to_square(player->x), to_square(player->y), player->explosion_radius, BOMB_TICK_LIFETIME, game->map);
                        player->bomb_count++;
                        break;
                    }
                }
            }
            break;
        case PA_KICK_BOMB:
            get_square_in_direction(player->x, player->y, player->dir, &player_next_x, &player_next_y);
            bomb = Game_get_bomb_at_location(game, player_next_x, player_next_y);
            if (bomb != NO_BOMB_FOUND) {
                get_square_in_direction(from_square(player_next_x), from_square(player_next_y), player->dir, &bomb_next_x, &bomb_next_y);
                if (is_square_empty(bomb_next_x, bomb_next_y, game->map)) {
                    Bomb_move(game->bombs[bomb], bomb_next_x, bomb_next_y);
                }
            }
            break;
        default:
            break;
    }
}

void Game_start(Game *game) {
    // Load basic map
    window_set_background_color(game->window, GColorBlack);

    // Load tiles
    GBC_Graphics_load_entire_tilesheet_into_vram(game->graphics, RESOURCE_ID_BACKGROUND_TILESHEET, 0, BACKGROUND_VRAM);
    GBC_Graphics_set_bg_palette_array(game->graphics, 0, TILESHEET_PALETTES[0]);
    GBC_Graphics_bg_set_scroll_pos(game->graphics, 0, SCREEN_OFFSET.x, SCREEN_OFFSET.y);

    // Load sprites
    GBC_Graphics_load_entire_tilesheet_into_vram(game->graphics, RESOURCE_ID_PLAYER_SPRITESHEET, 0, PLAYER_VRAM);

    for (uint8_t i = 0; i < MAX_PLAYERS; i++) {
        game->players[i] = NULL;
    }
    for (uint8_t i = 0; i < game->num_players; i++) {
        PlayerTypes player_type = PT_HUMAN;
        switch (i) {
            case 1:
                player_type = PT_HUNTER;
                break;
            case 2:
                player_type = PT_FRIEND;
                break;
            case 3:
                player_type = PT_HOARDER;
                break;
            default:
                break;
        }
        // if (i >= game->num_humans) {
        //     player_type = PT_HUNTER;
        // }
        game->players[i] = Player_initialize(
            i, player_type, from_square(game->player_spawns[i * 2]), from_square(game->player_spawns[i * 2 + 1]), 
            game->graphics, game->map, Game_handle_player_action, (void *)game
        );
        set_square(game->player_spawns[i * 2], game->player_spawns[i * 2 + 1], game->map, T_NONE);
    }
    game->player_one = game->players[0];

    for (uint8_t i = 0; i < MAX_BOMBS_PER_GAME; i++) {
        game->bombs[i] = NULL;
    }
    for (uint8_t i = 0; i < MAX_EXPLOSIONS_PER_GAME; i++) {
        game->explosions[i] = NULL;
    }
    for (uint8_t i = 0; i < MAX_ROCKS_PER_GAME; i++) {
        game->rocks[i] = NULL;
    }
    for (uint8_t i = 0; i < MAX_ITEMS_PER_GAME; i++) {
        game->items[i] = NULL;
    }
    
    // Game_load_player(game, 0, player_data);
    // uint16_t player_x = 0; // ((GBC_Graphics_get_screen_width(game->graphics) / 2 - (PLAYER_SPRITE_WIDTH / 2)) / 8) * 8;
    // uint16_t player_y = 0; // ((GBC_Graphics_get_screen_height(game->graphics) / 2 - (PLAYER_SPRITE_HEIGHT / 2)) / 8) * 8;
    // Player_set_position(game->player_one, player_x, player_y, D_DOWN);

    game->in_focus = true;
    
    // Show game (enable lcdc bit)
    GBC_Graphics_lcdc_set_enabled(game->graphics, true);
    Game_render(game);
}

// void Game_set_player_position(Game *game, char *username, uint16_t x, uint16_t y, Direction dir, bool animate) {
//     int8_t player_number = Game_get_player_by_name(game, username);
//     if (player_number == -1) {
//         APP_LOG(APP_LOG_LEVEL_WARNING, "User not found: %s", username);
//         poll_rendered_users();
//     } else if (player_number != 0) {
//         Player *player = game->players[player_number];
//         if (!player->active) {
//             Player_activate(player);
//         }
//         if (animate) {
//             Player_walk_to_location(player, x, y, dir, W_STEP);
//         } else {
//             Player_set_position(player, x, y, dir);
//         }
//     }
// }

// Direction Game_calculate_tilt(Game *game) {
//     AccelData accel = (AccelData) { .x = 0, .y = 0, .z = 0 };
//     accel_service_peek(&accel);
//     int accel_x = accel.x - game->accel_cal_x;
//     int accel_y = accel.y - game->accel_cal_y;
//     if (abs(accel_x) > abs(accel_y)) {
//         if (accel_x < -TILT_THRESHOLD) {
//             return D_LEFT;
//         } else if (accel_x > TILT_THRESHOLD) {
//             return D_RIGHT;
//         }
//     } else {
//         if (accel_y < -TILT_THRESHOLD) {
//             return D_DOWN;
//         } else if (accel_y > TILT_THRESHOLD) {
//             return D_UP;
//         }
//     }
//     return D_MAX;
// }

void Game_calibrate_accel(Game *game) {
    AccelData accel = (AccelData) { .x = 0, .y = 0, .z = 0 };
    accel_service_peek(&accel);
    game->accel_cal_x = accel.x;
    game->accel_cal_y = accel.y;
    Player_set_tilt_direction(game->player_one, D_NONE);
}

void Game_get_explosion_bounds(Game *game, uint8_t x, uint8_t y, uint8_t *bounds, uint8_t explosion_radius) {
    for (uint8_t i = 0; i < NUM_DIRS; i++) {
        uint8_t expl_x = x;
        uint8_t expl_y = y;
        uint8_t next_x = x;
        uint8_t next_y = y;
        while (true) {
            next_x += X_Y_DIRS[i][0];
            next_y += X_Y_DIRS[i][1];
            if (!is_square_explodable(next_x, next_y, game->map)
                || abs((int8_t)x - next_x) > explosion_radius
                || abs((int8_t)y - next_y) > explosion_radius) {
                bounds[i*2] = expl_x;
                bounds[i*2 + 1] = expl_y;
                break;
            }
            expl_x = next_x;
            expl_y = next_y;
        }
    }
}

int8_t Game_get_bomb_at_location(Game *game, uint8_t x, uint8_t y) {
    for (uint8_t i = 0; i < MAX_BOMBS_PER_GAME; i++) {
        if (game->bombs[i] != NULL) {
            if (game->bombs[i]->x == x && game->bombs[i]->y == y) {
                return i;
            }
        }
    }
    return NO_BOMB_FOUND;
}

void Game_chain_detonate_bombs(Game *game, uint8_t x, uint8_t y) {
    int8_t next_bomb_idx = Game_get_bomb_at_location(game, x, y);
    if (next_bomb_idx != NO_BOMB_FOUND) {
        if (game->bombs[next_bomb_idx]->state != BS_DONE) {
            Bomb_explode(game->bombs[next_bomb_idx]);
            Game_explode_bomb(game, next_bomb_idx);
        }
    }
}

void Game_explode_bomb(Game *game, uint8_t bomb_idx) {
    Bomb *bomb =  game->bombs[bomb_idx];
    Player *player =  game->players[game->bombs[bomb_idx]->owner];
    for (uint8_t i = 0; i < MAX_EXPLOSIONS_PER_GAME; i++) {
        if (game->explosions[i] == NULL) {
            uint8_t bounds[NUM_DIRS * 2];
            Game_get_explosion_bounds(game, bomb->x, bomb->y, bounds, bomb->explosion_radius);
            game->explosions[i] = Explosion_initialize(bomb->x, bomb->y, bounds, bomb->explosion_radius, EXPL_TICK_LIFETIME, game->map);

            // Check if any bombs get exploded
            for (uint8_t x = bounds[D_LEFT*2]; x <= bounds[D_RIGHT*2]; x++) {
                Game_chain_detonate_bombs(game, x, bomb->y);
            }
            for (uint8_t y = bounds[D_UP*2+1]; y <= bounds[D_DOWN*2+1]; y++) {
                Game_chain_detonate_bombs(game, bomb->x, y);
            }
            
            // Then check if anything got blown up
            for (uint8_t i = 0; i < NUM_DIRS; i++) {
                uint8_t x = bounds[i*2];
                uint8_t y = bounds[i*2 + 1];

                if (abs((int8_t)bomb->x - x) != bomb->explosion_radius && abs((int8_t)bomb->y - y) != bomb->explosion_radius) {
                    x += X_Y_DIRS[i][0];
                    y += X_Y_DIRS[i][1];
                    if (is_square_destroyable(x, y, game->map)) {
                        // Blow up stuff
                        switch (game->map[x + y *SCREEN_BLOCK_WIDTH]) {
                            case T_ROCK:
                                for (uint8_t i = 0; i < MAX_ROCKS_PER_GAME; i++) {
                                    if (game->rocks[i] == NULL) {
                                        game->rocks[i] = Rock_initialize(x, y, game->map);
                                        break;
                                    }
                                }
                                break;
                            case T_ITEM_BOMB:
                            case T_ITEM_RANGE:
                            case T_ITEM_SHIELD:
                            case T_ITEM_KICK:
                            case T_ITEM_POWER:
                                for (uint8_t i = 0; i < MAX_ITEMS_PER_GAME; i++) {
                                    if (game->items[i] == NULL) {
                                        game->items[i] = Item_initialize(x, y, game->map);
                                        break;
                                    }
                                }
                                break;
                            default:
                                set_square(x, y, game->map, T_NONE);
                                break;
                        }
                    }
                }
            }
            break;
        }
    }
    game->players[game->bombs[bomb_idx]->owner]->bomb_count--;
    Bomb_destroy(game->bombs[bomb_idx]);
    game->bombs[bomb_idx] = NULL;
}

void Game_calc_scores(Game *game) {
    memset(game->scores, SCORE_NULL, sizeof(game->scores));
    uint8_t block, posn;
    int8_t score;
    // First go by block
    for (uint8_t y = 0; y < SCREEN_BLOCK_HEIGHT; y++) {
        for (uint8_t x = 0; x < SCREEN_BLOCK_WIDTH; x++) {
            posn = x + y * SCREEN_BLOCK_WIDTH;
            block = game->map[posn];
            score = TILES_TO_SCORE[block];
            game->scores[posn] = score;
        }
    }
    // Now apply bombs
    for (uint8_t i = 0; i < MAX_BOMBS_PER_GAME; i++) {
        if (game->bombs[i] != NULL) {
            uint8_t bounds[NUM_DIRS * 2];
            Bomb *bomb = game->bombs[i];
            uint8_t bomb_base_score = (bomb->time_left / FRAMERATE) + SCORE_BOMB;
            Game_get_explosion_bounds(game, bomb->x, bomb->y, bounds, bomb->explosion_radius);

            // Calculate scores
            for (uint8_t x = bounds[D_LEFT*2]; x <= bounds[D_RIGHT*2]; x++) {
                posn = x + bomb->y * SCREEN_BLOCK_WIDTH;
                score = bomb_base_score + abs(x - bomb->x);
                if (score < game->scores[posn]) {
                    game->scores[posn] = score;
                }
            }
            for (uint8_t y = bounds[D_UP*2+1]; y <= bounds[D_DOWN*2+1]; y++) {
                posn = bomb->x + y * SCREEN_BLOCK_WIDTH;
                score = bomb_base_score + abs(y - bomb->y);
                if (score < game->scores[posn]) {
                    game->scores[posn] = score;
                }
            }
        }
    }

#if DEBUG_SCORES
    // Print it for debug!!
    APP_LOG(APP_LOG_LEVEL_DEBUG, "> ---------");
    for (uint8_t y = 0; y < SCREEN_BLOCK_HEIGHT; y++) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "%3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d",
            game->scores[0 + y * SCREEN_BLOCK_WIDTH],
            game->scores[1 + y * SCREEN_BLOCK_WIDTH],
            game->scores[2 + y * SCREEN_BLOCK_WIDTH],
            game->scores[3 + y * SCREEN_BLOCK_WIDTH],
            game->scores[4 + y * SCREEN_BLOCK_WIDTH],
            game->scores[5 + y * SCREEN_BLOCK_WIDTH],
            game->scores[6 + y * SCREEN_BLOCK_WIDTH],
            game->scores[7 + y * SCREEN_BLOCK_WIDTH],
            game->scores[8 + y * SCREEN_BLOCK_WIDTH],
            game->scores[9 + y * SCREEN_BLOCK_WIDTH],
            game->scores[10 + y * SCREEN_BLOCK_WIDTH],
            game->scores[11 + y * SCREEN_BLOCK_WIDTH]
        );
    }
    APP_LOG(APP_LOG_LEVEL_DEBUG, "--------- <");
#endif // DEBUG_SCORES
}

int8_t Game_get_closest_player(Game *game, Player *player, HuntTypes hunt_type) {
    int16_t min_distance = 1000;
    int16_t checked_distance;
    int8_t closest_player = NO_OTHER_PLAYERS;
    for (uint8_t i = 0; i < game->num_players; i++) {
        if (i != player->number && game->players[i] != NULL && game->players[i]->state != P_DEAD) {
            checked_distance = manhattan_distance(
                to_square(game->players[i]->x), to_square(game->players[i]->y),
                to_square(player->x), to_square(player->y)
            );
            if (checked_distance <= min_distance) {
                switch (hunt_type) {
                    case HT_HUMAN:
                        if (game->players[i]->player_type != PT_HUMAN) {
                            continue;
                        }
                        break;
                    case HT_CPU:
                        if (game->players[i]->player_type == PT_HUMAN) {
                            continue;
                        }
                        break;
                    default:
                        break;
                }
                if (closest_player == NO_OTHER_PLAYERS || (checked_distance == min_distance && (rand() % 2 == 0))) {
                    closest_player = i;
                    min_distance = checked_distance;
                }
            }
        }
    }
    return closest_player;
}

bool Game_get_closest_tile_of_score(Game *game, Player *player, int8_t target_score, uint8_t *out_x, uint8_t *out_y) {
    int16_t min_distance = 1000;
    int16_t checked_distance;
    bool found_score = false;
    for (uint8_t x = 0; x < SCREEN_BLOCK_WIDTH; x++) {
        for (uint8_t y = 0; y < SCREEN_BLOCK_HEIGHT; y++) {
            if (game->scores[x + y * SCREEN_BLOCK_WIDTH] == target_score) {
                checked_distance = manhattan_distance(
                    x, y,
                    to_square(player->x), to_square(player->y)
                );
                if (checked_distance < min_distance) {
                    *out_x = x;
                    *out_y = y;
                    min_distance = checked_distance;
                    found_score = true;
                }
            }
        }
    }
    return found_score;
}

void Game_get_safest_adjacent_tile(Game *game, Player *player, uint8_t *out_x, uint8_t *out_y) {
    int8_t highest_score = SCORE_WALL;
    uint8_t next_x, next_y;
    int8_t score;
    for (uint8_t i = 0; i < NUM_DIRS + 1; i++) {
        next_x = to_square(player->x) + X_Y_DIRS[i][0];
        next_y = to_square(player->y) + X_Y_DIRS[i][1];
        score = game->scores[next_x + next_y * SCREEN_BLOCK_WIDTH];
        if (score > highest_score || (score == highest_score && ((i == D_NONE) || (rand() % 2 == 0)))) {
            highest_score = score;
            *out_x = next_x;
            *out_y = next_y;
        }
    }
}

bool Game_player_on_square(Game *game, int8_t x, int8_t y) {
    for (uint8_t i = 0; i < game->num_players; i++) {
        if (game->players[i] != NULL) {
            if (to_square(game->players[i]->x) == x && to_square(game->players[i]->y) == y) {
                return true;
            }
        }
    }
    return false;
}

void Game_control_cpus(Game *game) {
    // TODO: based on CPU personality, pick a target
    // PT_HUMAN, // Me and you
    // PT_COWARD, // Prioritize safety
    // PT_HUNTER, // Prioritize humans
    // PT_LAZY, // Takes longer to think
    // PT_HOARDER, // Prioritizes items
    // PT_DEMO, // Prioritizes rocks
    // PT_FRIEND, // Prioritizes CPUs
    // PT_SMART, // Prioritizes by path score: items, players

    uint8_t target_squares[MAX_PLAYERS][2];
    int8_t closest_player;
    for (uint8_t i = 0; i < game->num_players; i++) {
        if (game->players[i] != NULL && Player_cpu_ready(game->players[i])) {
            // Pick target
            // By default, target closest player
            bool found_path = false;
            // if not safe, target safest, cloest square 
            // - invincible means all squares are safe!
            // maybe for most, pick the square from the 3x3 around
            // For coward, pick ay safe square in a 5x5?
            // otherwise: hunt!

            uint8_t adj_x, adj_y;
            Game_get_safest_adjacent_tile(game, game->players[i], &adj_x, &adj_y);
            bool square_is_safe = is_square_safe(to_square(game->players[i]->x), to_square(game->players[i]->y), game->scores, game->players[i]->invincibility_frames > 0);
            bool next_square_is_safer = is_next_square_safer(to_square(game->players[i]->x), to_square(game->players[i]->y), adj_x, adj_y, game->scores);
            // checking safer allows for adjacent item detours
            if ((square_is_safe && next_square_is_safer) || !square_is_safe) {
                target_squares[i][0] = adj_x;
                target_squares[i][1] = adj_y;
                found_path = true;
            }
            if (!found_path) {
                switch (game->players[i]->player_type) {
                    case PT_HUNTER:
                        closest_player = Game_get_closest_player(game, game->players[i], HT_HUMAN);
                        if (closest_player != -1) {
                            target_squares[i][0] = to_square(game->players[closest_player]->x);
                            target_squares[i][1] = to_square(game->players[closest_player]->y);
                            found_path = true;
                        }
                        break;
                    case PT_FRIEND:
                        closest_player = Game_get_closest_player(game, game->players[i], HT_CPU);
                        if (closest_player != NO_OTHER_PLAYERS) {
                            target_squares[i][0] = to_square(game->players[closest_player]->x);
                            target_squares[i][1] = to_square(game->players[closest_player]->y);
                            found_path = true;
                        }
                        break;
                    case PT_HOARDER:
                        if (Game_get_closest_tile_of_score(game, game->players[i], SCORE_ITEM, &target_squares[i][0], &target_squares[i][1])) {
                            found_path = true;
                        }
                        break;
                    default:
                        target_squares[i][0] = to_square(game->players[i]->x);
                        target_squares[i][1] = to_square(game->players[i]->y);
                        break;
                }
            }
            // Couldn't find special path? Go towards the closest player
            if (!found_path) {
                closest_player = Game_get_closest_player(game, game->players[i], HT_PLAYER);
                if (closest_player != NO_OTHER_PLAYERS) {
                    target_squares[i][0] = to_square(game->players[closest_player]->x);
                    target_squares[i][1] = to_square(game->players[closest_player]->y);
                    found_path = true;
                } else { // No players at all??? Well go to yourself I guess
                    target_squares[i][0] = to_square(game->players[i]->x);
                    target_squares[i][1] = to_square(game->players[i]->y);
                    found_path = true;
                }
            }

            if (found_path) {
                // Find path
                memset(game->cpu_paths[i], -1, sizeof(game->cpu_paths[i]));
                game->cpu_path_lens[i] = 0;
                game->cpu_path_scores[i] = find_path(to_square(game->players[i]->x), to_square(game->players[i]->y),
                        target_squares[i][0], target_squares[i][1],
                        game->map, game->cpu_paths[i], &game->cpu_path_lens[i]);
            }

            // Decide what to do
            // square open and safe? move there
            // square blocked by rock? place bomb
            // Get the next square
            int8_t cur_x = to_square(game->players[i]->x);
            int8_t cur_y = to_square(game->players[i]->y);
            int8_t next_x = game->cpu_paths[i][1 * 2];
            int8_t next_y = game->cpu_paths[i][1 * 2 + 1];
            // uint8_t next_valid_idx = get_next_valid_index(game->cpu_paths[i], game->cpu_path_lens[i]);
            // if (next_valid_idx == 0) {
            //     printf("Start is next valid square!!");
            // }
            // int8_t next_x = game->cpu_paths[i][next_valid_idx * 2];
            // int8_t next_y = game->cpu_paths[i][next_valid_idx * 2 + 1];
            square_is_safe = is_square_safe(next_x, next_y, game->scores, game->players[i]->invincibility_frames > 0);
            next_square_is_safer = is_next_square_safer(cur_x, cur_y, next_x, next_y, game->scores);
            bool next_square_ok = (
                !is_square_blocking(next_x, next_y, game->map, game->players[i]->invincibility_frames > 0)
                && (next_square_is_safer || (square_is_safe && !Game_player_on_square(game, next_x, next_y)))
                && ((next_x != -1) && (next_y != -1))
            );
            if (next_square_ok) {
                if ((abs(next_x - cur_x) > 1) || (abs(next_y - cur_y) > 1)) {
                    APP_LOG(APP_LOG_LEVEL_DEBUG, "Jumped! (%d, %d) -> (%d, %d)", cur_x, cur_y, next_x, next_y);
                }
                Player_walk_to_location(game->players[i], from_square(next_x), from_square(next_y), pos_delta_to_dir(next_x - cur_x, next_y - cur_y));
            } else if (game->map[next_x + next_y * SCREEN_BLOCK_WIDTH] == T_ROCK || Game_player_on_square(game, next_x, next_y)) {
                Player_place_bomb(game->players[i]);
            } else {
                Player_set_direction(game->players[i], pos_delta_to_dir(next_x - cur_x, next_y - cur_y));
            }
        }
    }

#if DEBUG_PATHS
    for (uint8_t i = game->num_humans; i < game->num_players; i++) {
        // Print it for debug!!
        if (game->cpu_path_scores[i] >= 0) {
            APP_LOG(APP_LOG_LEVEL_DEBUG, "> Path %d | len %d | score %d ----", i, game->cpu_path_lens[i], game->cpu_path_scores[i]);
            for (uint8_t y = 1; y < SCREEN_BLOCK_HEIGHT - 1; y++) {
                APP_LOG(APP_LOG_LEVEL_DEBUG, "| %c | %c | %c | %c | %c | %c | %c | %c | %c |",
                    // path_contains_point(0, y, game->cpu_paths[i], game->cpu_path_lens[i]) ? 'x' : ' ',
                    path_contains_point(1, y, game->cpu_paths[i], game->cpu_path_lens[i]) ? 'x' : ' ',
                    path_contains_point(2, y, game->cpu_paths[i], game->cpu_path_lens[i]) ? 'x' : ' ',
                    path_contains_point(3, y, game->cpu_paths[i], game->cpu_path_lens[i]) ? 'x' : ' ',
                    path_contains_point(4, y, game->cpu_paths[i], game->cpu_path_lens[i]) ? 'x' : ' ',
                    path_contains_point(5, y, game->cpu_paths[i], game->cpu_path_lens[i]) ? 'x' : ' ',
                    path_contains_point(6, y, game->cpu_paths[i], game->cpu_path_lens[i]) ? 'x' : ' ',
                    path_contains_point(7, y, game->cpu_paths[i], game->cpu_path_lens[i]) ? 'x' : ' ',
                    path_contains_point(8, y, game->cpu_paths[i], game->cpu_path_lens[i]) ? 'x' : ' ',
                    path_contains_point(9, y, game->cpu_paths[i], game->cpu_path_lens[i]) ? 'x' : ' ',
                    path_contains_point(10, y, game->cpu_paths[i], game->cpu_path_lens[i]) ? 'x' : ' '
                    // path_contains_point(11, y, game->cpu_paths[i], game->cpu_path_lens[i]) ? 'x' : ' '
                );
            }
            APP_LOG(APP_LOG_LEVEL_DEBUG, "----------------------------- <");
        } else {
            APP_LOG(APP_LOG_LEVEL_DEBUG, "> !!! Failed to find path %d ----", i);
            APP_LOG(APP_LOG_LEVEL_DEBUG, "----------------------------- <");
        } 
    }
#endif // DEBUG_PATHS
}

bool Game_step(Game *game) {
    if (window_stack_get_top_window() == game->window && game->in_focus) {
        if (game->paused) {
            Game_calibrate_accel(game);
            game->paused = false;
        }
    } else {
        if (!game->paused) {
            game->paused = true;
        }
    }
    if (!game->paused) {
        // if (game->settings->Tilt) {
        //     Player_set_tilt_direction(game->player_one, Game_calculate_tilt(game));
        // }
        // Erase blank spaces
        for (uint8_t x = 0; x < SCREEN_BLOCK_WIDTH; x++) {
            for (uint8_t y = 0; y < SCREEN_BLOCK_HEIGHT; y++) {
                TileType block = game->map[x + y * SCREEN_BLOCK_WIDTH];
                if (block >= T_EXPL_WEST_EDGE) {
                    set_square(x, y, game->map, T_NONE);
                }
            }
        }
        // Bombs
        for (uint8_t i = 0; i < MAX_BOMBS_PER_GAME; i++) {
            if (game->bombs[i] != NULL) {
                Bomb_step(game->bombs[i]);
                if (game->bombs[i]->state == BS_DONE) {
                    Game_explode_bomb(game, i);
                }
            }
        }
        // Explosions
        game->explosion_frame = (game->explosion_frame + 1) % 2;
        for (uint8_t i = 0; i < MAX_EXPLOSIONS_PER_GAME; i++) {
            if (game->explosions[i] != NULL) {
                Explosion_step(game->explosions[i]);
                if (game->explosions[i]->state == ES_DONE) {
                    Explosion_destroy(game->explosions[i]);
                    game->explosions[i] = NULL;
                }
            }
        }
        for (uint8_t i = 0; i < MAX_EXPLOSIONS_PER_GAME; i++) {
            if (game->explosions[i] != NULL) {
                Explosion_render(game->explosions[i], game->explosion_frame);
            }
        }
        // Rocks (destroyed)
        for (uint8_t i = 0; i < MAX_ROCKS_PER_GAME; i++) {
            if (game->rocks[i] != NULL) {
                Rock_step(game->rocks[i]);
                if (game->rocks[i]->state == RS_DONE) {
                    uint8_t item_spawn_roll = rand() % ITEM_SPAWN_RANGE;
                    if (item_spawn_roll < NUM_ITEMS) {
                        set_square(game->rocks[i]->x, game->rocks[i]->y, game->map, T_ITEM_BOMB + item_spawn_roll);
                    }
                    Rock_destroy(game->rocks[i]);
                    game->rocks[i] = NULL;
                }
            }
        }
        // Items (destroyed)
        for (uint8_t i = 0; i < MAX_ITEMS_PER_GAME; i++) {
            if (game->items[i] != NULL) {
                Item_step(game->items[i]);
                if (game->items[i]->state == IS_DONE) {
                    Item_destroy(game->items[i]);
                    game->items[i] = NULL;
                }
            }
        }

        // Calculate map scores
        Game_calc_scores(game);

        game->path_tick = (game->path_tick + 1) % 1;
        if (game->path_tick == 0) {
            Game_control_cpus(game);
        }

        // CPU logic
        // want some kind of frame tick, cpus shouldn't action every tick, in player class
        // 1. turn in direction if not already
        // 2. depending on next square, move or drop bomb
        // 3. ... is that all?

        bool game_over = false;
        uint8_t alive_player_count = 0;
        // Players
        for (uint8_t i = 0; i < game->num_players; i++) {
            Player_step(game->players[i]);
            if (game->players[i]->state != P_DEAD) {
                alive_player_count += 1;
            }
        }
        game_over = alive_player_count <= 1 || (game->game_type == GT_VS_CPU && game->player_one->state == P_DEAD);
        if (game_over) {
            return false;
        }
        for (uint8_t i = 0; i < game->num_players; i++) {
            Player_render(game->players[i]);
        }
        // (explosions, items)
        // Bombs, broken rocks)
        Game_render(game);
        GBC_Graphics_render(game->graphics);
    }
    return true;
}

void Game_input_handler(Game *game, QueuedInput input) {
    // for (uint8_t i = 0; i < game->num_players; i++) {
    //     Player_push_input(game->players[i], input);
    // }
    Player_push_input(game->player_one, input);
}

void Game_up_handler(Game *game) {
    // if (game->player_one->settings->Tilt) {
    //     Game_calibrate_accel(game);
    // } else {
    {
        Player_push_input(game->player_one, Q_UP);
    }
}

void Game_down_handler(Game *game) {
    // if (game->player_one->settings->Tilt) {
    //     Game_calibrate_accel(game);
    // } else {
    {
        Player_push_input(game->player_one, Q_DOWN);
    }
}

void Game_select_handler(Game *game, QueuedInput input) {
    // Step or interact or stop walk
    Player_push_input(game->player_one, input);
}

void Game_up_long_handler(Game *game) {
    // Player_stop_moving(game->player_one);
    // Player_stop_awaiting(game->player_one);
    // if (game->hold_popup != NULL) {
    //     StaticPopup_destroy(game->hold_popup);
    //     game->hold_popup = NULL;
    // }
    // Game_clear_notif(game);
    // game->main_menu = MainMenu_init(game->players, game->settings, Game_release_main_menu, (void *) game);
    // if (game->settings->OpenedMenu == false) {
    //     game->settings->OpenedMenu = true;
    //     save_settings(game->settings);
    // }
    // for (uint8_t i = 0; i < MAX_PLAYERS; i++) {
    //     Player_destroy_username_text(game->players[i]);
    // }
    // s_menu_null = false;
}

void Game_down_long_handler(Game *game) {
    // static uint8_t zoom_level;
    // zoom_level = (zoom_level + 1) % 3;
    // if (game->zoom_popup != NULL) {
    //     StaticPopup_destroy(game->zoom_popup);
    //     game->zoom_popup = NULL;
    // }
    // if (game->settings->Zoomed == false) {
    //     game->settings->Zoomed = true;
    //     save_settings(game->settings);
    // }

    // GBC_Graphics_zoom_set_level(game->graphics, zoom_level);
}

void Game_back_handler(Game *game) {
    // bomb
    Player_push_input(game->player_one, Q_BACK);
}

// void Game_tap_handler(Game *game) {
    
// }

void Game_focus_handler(Game *game, bool in_focus) {
    game->in_focus = in_focus;
}
