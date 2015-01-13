#include "pebble.h"

#define STATUS_LINE_TIMEOUT 5000
 
static Window *window;
static GBitmap *background_bitmap;
static GBitmap *hour_bitmap;
static GBitmap *minute_bitmap;
static GBitmap *minute_hour_bitmap;
static BitmapLayer *background_layer;
static BitmapLayer *hour_layer;
static BitmapLayer *minute_layer;
static BitmapLayer *minute_hour_layer;
static TextLayer *date_layer;
static TextLayer *time_layer;
static GFont *extra_font;
static GFont *extra_font_large;
bool bt_connect_toggle;
static AppTimer *shake_timeout = NULL;
static char mytime[] = "00:00";
static char stat[] = " ¦100% § \n\n\nWed 31   ";
struct tm *t;


const uint8_t const X[] = {
   65,
   98,
  122,
  131,
  122,
   98,
   65,
   34,
    8,
    0,
    8,
   32
};

const uint8_t const Y[] = {
   11,
   20,
   44,
   77,
  110,
  134,
  143,
  134,
  110,
   77,
   44,
   20
};

void bluetooth_connection_handler(bool connected) {
  if (!bt_connect_toggle && connected) {
    bt_connect_toggle = true;
    vibes_short_pulse();
  }
  if (bt_connect_toggle && !connected) {
    bt_connect_toggle = false;
    vibes_short_pulse();
  }
}

void show_extra (void *isShow) {
  static char mydate[] = "wed 31";
  BatteryChargeState charge_state = battery_state_service_peek();
  
  if (isShow) {
    if(clock_is_24h_style() == true) { 
      strftime(mytime, sizeof(mytime), "%H:%M", t);
    } else {
      strftime(mytime, sizeof(mytime), "%I:%M", t);
    }

    strftime(mydate, sizeof(mydate), "%a %e", t);
    if (bt_connect_toggle) {
      snprintf(stat, sizeof(stat), "¦%d%%\n\n\n%s", charge_state.charge_percent, mydate);
    }
    else {
      snprintf(stat, sizeof(stat), "¦%d%% §\n\n\n%s", charge_state.charge_percent, mydate);
    }

    text_layer_set_text(time_layer, mytime);
    text_layer_set_text(date_layer, stat);
  }
  layer_set_hidden(text_layer_get_layer(time_layer), !isShow);
  layer_set_hidden(text_layer_get_layer(date_layer), !isShow);
  shake_timeout=NULL;
}
void wrist_flick_handler(AccelAxisType axis, int32_t direction) {
  if (axis == 1 && !shake_timeout) {
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "SHAKE IT");
    show_extra((void *)true);
    shake_timeout = app_timer_register (STATUS_LINE_TIMEOUT, show_extra, (AppTimerCallback)false);
  }
}


void move_dots (int hours, int minutes, int seconds) {
  // We want to operate with a resolution of 30 seconds.  So multiply
  // minutes and seconds by 2.  Then divide by (2 * 5) to carve the hour
  // into five minute intervals.
  int half_mins  = (2 * minutes) + (seconds / 30);
  int minute_index  = ((half_mins + 5) / (2 * 5)) % 12;
  int hour_index;

  if (minute_index == 0 && minutes > 30) {
    hour_index = (hours + 1) % 12;
  }
  else {
    hour_index = hours % 12;
  }

  if (hour_index != minute_index) {
    layer_set_frame(bitmap_layer_get_layer(hour_layer), GRect(X[hour_index], Y[hour_index], 13, 13));
    layer_mark_dirty(bitmap_layer_get_layer(hour_layer));
    layer_set_frame(bitmap_layer_get_layer(minute_layer), GRect(X[minute_index], Y[minute_index], 13, 13));
    layer_mark_dirty(bitmap_layer_get_layer(minute_layer));
    layer_set_hidden(bitmap_layer_get_layer(hour_layer), false);
    layer_set_hidden(bitmap_layer_get_layer(minute_layer), false);
    layer_set_hidden(bitmap_layer_get_layer(minute_hour_layer), true);
  }
  else {
    layer_set_hidden(bitmap_layer_get_layer(hour_layer), true);
    layer_set_hidden(bitmap_layer_get_layer(minute_layer), true);
    layer_set_frame(bitmap_layer_get_layer(minute_hour_layer), GRect(X[minute_index], Y[minute_index], 13, 13));
    layer_set_hidden(bitmap_layer_get_layer(minute_hour_layer), false);
  }
}

static void update_time(struct tm *t, TimeUnits units_changed) {
 
  move_dots (t->tm_hour, t->tm_min, t->tm_sec);
}

static void window_load(Window *window) {
  extra_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_EXTRA_16));
  extra_font_large = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_EXTRA_32));

  background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND);
  background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(background_layer , background_bitmap);
  
  hour_bitmap = gbitmap_create_with_resource(RESOURCE_ID_HOUR);
  hour_layer = bitmap_layer_create(GRect(X[0], Y[0], 13, 13));
  bitmap_layer_set_bitmap(hour_layer , hour_bitmap);

  minute_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MINUTE);
  minute_layer = bitmap_layer_create(GRect(X[1], Y[1], 13, 13));
  bitmap_layer_set_bitmap(minute_layer , minute_bitmap);

  minute_hour_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MINUTE_HOUR);
  minute_hour_layer = bitmap_layer_create(GRect(X[0], Y[0], 13, 13));
  bitmap_layer_set_bitmap(minute_hour_layer , minute_hour_bitmap);
  
  //date_layer = text_layer_create(GRect(0, 40, 144, 80));
  date_layer = text_layer_create(GRect(0, 47, 144, 80));
  text_layer_set_text_color(date_layer, GColorWhite);
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_font(date_layer, extra_font);
  text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);

  time_layer = text_layer_create(GRect(0, 60, 144, 80));
  text_layer_set_text_color(time_layer, GColorWhite);
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_font(time_layer, extra_font_large);
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);

  
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(background_layer));
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(hour_layer));
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(minute_layer));
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(minute_hour_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(date_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(time_layer));
  layer_set_hidden(bitmap_layer_get_layer(minute_hour_layer), true);
  layer_set_hidden(text_layer_get_layer(time_layer), true);
  layer_set_hidden(text_layer_get_layer(date_layer), true);
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
  text_layer_destroy(date_layer);
  text_layer_destroy(time_layer);
  fonts_unload_custom_font(extra_font);
  fonts_unload_custom_font(extra_font_large);
}

static void init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });
  window_stack_push(window, true /* Animated */);

  bluetooth_connection_service_subscribe(bluetooth_connection_handler);
  bt_connect_toggle = bluetooth_connection_service_peek();

  accel_tap_service_subscribe(wrist_flick_handler);
  accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);

  tick_timer_service_subscribe(MINUTE_UNIT, update_time);

  time_t temp = time(NULL);
  t = localtime(&temp);
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