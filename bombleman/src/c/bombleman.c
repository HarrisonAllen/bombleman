#include <pebble.h>
#include "game.h"
#include "pebble-gbc-graphics-advanced/pebble-gbc-graphics-advanced.h"

static Window *s_main_window;

static GBC_Graphics *s_gbc_graphics;
static Game *s_game;
static AppState s_state = S_MENUS;
static AppTimer *s_frame_timer;
static bool mid_frame;

static void end_game();
static void start_game();

static void frame_timer_handle(void* context) {
  if (mid_frame) return;
  mid_frame = true;
  s_frame_timer = app_timer_register(FRAME_DURATION, frame_timer_handle, NULL); // WARNING: this may add instability if everything else can't finish. If so, move to the end of this function
  // APP_LOG(APP_LOG_LEVEL_DEBUG, "!%d", heap_bytes_free());
  if (!Game_step(s_game)) {
    end_game();
    start_game();
  }
  mid_frame = false;
}

static void start_game() {
  s_state = S_PLAY;
  s_game = Game_init(s_gbc_graphics, s_main_window, GT_VS_CPU, MAX_PLAYERS, 1);
  Game_start(s_game);
  s_frame_timer = app_timer_register(FRAME_DURATION, frame_timer_handle, NULL);
}

static void end_game() {
  Game_destroy(s_game);
  s_game = NULL;
  s_state = S_MENUS;
  if (s_frame_timer != NULL) {
    app_timer_cancel(s_frame_timer);
    s_frame_timer = NULL;
  }
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_state == S_PLAY) {
    Game_input_handler(s_game, Q_SELECT);
  }
}

static void select_press_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_state == S_PLAY) {
    Game_input_handler(s_game, Q_SELECT_PRESS);
  }
}

static void select_release_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_state == S_PLAY) {
    Game_input_handler(s_game, Q_SELECT_RELEASE);
  }
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_state == S_PLAY) {
    Game_input_handler(s_game, Q_UP);
  }
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_state == S_PLAY) {
    Game_input_handler(s_game, Q_DOWN);
  }
}

static void back_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_state == S_PLAY) {
    Game_input_handler(s_game, Q_BACK);
  }
}

static void click_config_provider(void *context) {
  // Clicks
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
  // Raw
  window_raw_click_subscribe(BUTTON_ID_SELECT, select_press_handler, select_release_handler, NULL);
}

static void will_focus_handler(bool in_focus) {
  if (s_state == S_PLAY) {
    if (!in_focus) {
      // If a notification pops up while the timer is firing
      // very rapidly, it will crash the entire watch :)
      // Stopping the timer when a notification appears will
      // prevent this while also pausing the gameplay
      if (s_frame_timer != NULL) {
        app_timer_cancel(s_frame_timer);
      }
    } else {
      if (s_frame_timer != NULL) {
        s_frame_timer = app_timer_register(FRAME_DURATION, frame_timer_handle, NULL);
      }
    }
    Game_focus_handler(s_game, in_focus);
  }
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  window_set_background_color(s_main_window, GColorWhite);

  // GBC Graphics Layer
  // Note, this creator adds the layer to the window, meaning if it's fired
  // first then it will be on the bottom
  uint16_t vram_sizes[] = {BACKGROUND_NUM_TILES * GBC_TILE_NUM_BYTES, PLAYER_NUM_TILES * GBC_TILE_NUM_BYTES};
  s_gbc_graphics = GBC_Graphics_ctor(s_main_window, NUM_VRAMS, vram_sizes, NUM_BACKGROUNDS);
  GBC_Graphics_set_screen_bounds(s_gbc_graphics, SCREEN_BOUNDS);
  GBC_Graphics_lcdc_set_enabled(s_gbc_graphics, false);
  
  start_game();
}

static void main_window_unload(Window *window) {
  if (s_game != NULL)
    Game_destroy(s_game);
  if (s_gbc_graphics != NULL)
    GBC_Graphics_destroy(s_gbc_graphics);
}

static void init() {
  // setup window
  s_main_window = window_create();

  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  window_stack_push(s_main_window, true);

  window_set_click_config_provider(s_main_window, click_config_provider);

  app_focus_service_subscribe(will_focus_handler);
}

static void deinit() {
  if (s_main_window != NULL)
    window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}