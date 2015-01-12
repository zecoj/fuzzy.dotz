#include "pebble.h"

static Window *window;

static GBitmap *background_bitmap;
static GBitmap *hour_bitmap;
static GBitmap *minute_bitmap;
static GBitmap *minute_hour_bitmap;
static BitmapLayer *background_layer;
static BitmapLayer *hour_layer;
static BitmapLayer *minute_layer;
static BitmapLayer *minute_hour_layer;

const uint8_t const X[] = {
   66,
   99,
  123,
  133,
  123,
   99,
   66,
   35,
    9,
    0,
    9,
   33
};

const uint8_t const Y[] = {
   12,
   21,
   45,
   78,
  111,
  135,
  144,
  135,
  111,
   78,
   45,
   21
};

void move_dots (int hours, int minutes, int seconds) {
  // We want to operate with a resolution of 30 seconds.  So multiply
  // minutes and seconds by 2.  Then divide by (2 * 5) to carve the hour
  // into five minute intervals.
  int half_mins  = (2 * minutes) + (seconds / 30);
  int minute_index  = ((half_mins + 5) / (2 * 5)) % 12;
  int hour_index = hours % 12;

  if (hour_index != minute_index) {
    layer_set_frame(bitmap_layer_get_layer(hour_layer), GRect(X[hour_index], Y[hour_index], 11, 11));
    layer_mark_dirty(bitmap_layer_get_layer(hour_layer));
    layer_set_frame(bitmap_layer_get_layer(minute_layer), GRect(X[minute_index], Y[minute_index], 11, 11));
    layer_mark_dirty(bitmap_layer_get_layer(minute_layer));
    layer_set_hidden(bitmap_layer_get_layer(hour_layer), false);
    layer_set_hidden(bitmap_layer_get_layer(minute_layer), false);
    layer_set_hidden(bitmap_layer_get_layer(minute_hour_layer), true);
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "hour_index:%d minute_index:%d", hour_index, minute_index);
  }
  else {
    layer_set_hidden(bitmap_layer_get_layer(hour_layer), true);
    layer_set_hidden(bitmap_layer_get_layer(minute_layer), true);
    layer_set_frame(bitmap_layer_get_layer(minute_hour_layer), GRect(X[minute_index], Y[minute_index], 11, 11));
    layer_set_hidden(bitmap_layer_get_layer(minute_hour_layer), false);
  }
}

static void update_time(struct tm *t, TimeUnits units_changed) {
  move_dots (t->tm_hour, t->tm_min, t->tm_sec);
}

static void window_load(Window *window) {
  background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND);
  background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(background_layer , background_bitmap);
  
  hour_bitmap = gbitmap_create_with_resource(RESOURCE_ID_HOUR);
  hour_layer = bitmap_layer_create(GRect(X[0], Y[0], 11, 11));
  bitmap_layer_set_bitmap(hour_layer , hour_bitmap);

  minute_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MINUTE);
  minute_layer = bitmap_layer_create(GRect(X[1], Y[1], 11, 11));
  bitmap_layer_set_bitmap(minute_layer , minute_bitmap);

  minute_hour_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MINUTE_HOUR);
  minute_hour_layer = bitmap_layer_create(GRect(X[0], Y[0], 11, 11));
  bitmap_layer_set_bitmap(minute_hour_layer , minute_hour_bitmap);
  
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(background_layer));
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(hour_layer));
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(minute_layer));
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(minute_hour_layer));
  layer_set_hidden(bitmap_layer_get_layer(minute_hour_layer), true);
}

static void window_unload(Window *window) {
  bitmap_layer_destroy(background_layer);
  bitmap_layer_destroy(hour_layer);
  bitmap_layer_destroy(minute_layer);
  bitmap_layer_destroy(minute_hour_layer);
  gbitmap_destroy(background_bitmap);
  gbitmap_destroy(hour_bitmap);
  gbitmap_destroy(minute_bitmap);
  gbitmap_destroy(minute_hour_bitmap);
}

static void init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });
  window_stack_push(window, true /* Animated */);
  
  tick_timer_service_subscribe(MINUTE_UNIT, update_time);
  
  time_t temp = time(NULL);
  struct tm *t = localtime(&temp);
  move_dots (t->tm_hour, t->tm_min, t->tm_sec);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}