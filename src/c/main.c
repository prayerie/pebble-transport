#include <pebble.h>
//#include "main.h"

#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1
#define SETTINGS_KEY 1

static Window *s_main_window;

static TextLayer *s_time_layer;
static TextLayer *s_bat_layer;
static TextLayer *s_dat_layer;
static TextLayer *s_wea_layer;
static TextLayer *s_tem_layer;
static TextLayer *s_wea_tem_layer;

static GFont transport_med_small;
static GFont transport_med;
static GFont transport_he;

static GFont motorway;

static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;
static GBitmap *s_background_filled_bitmap;

static BitmapLayer *s_background_layer, *s_bt_icon_layer;
static GBitmap *s_background_bitmap, *s_bt_icon_bitmap;

static int s_battery_level;

typedef struct ClaySettings {
    bool fahrenheit;
    bool fillCorners;
    bool twoLineWeather;
    char customKey[32];
    char customLoc[32];
    char colourScheme[32];
} __attribute__((__packed__)) ClaySettings;

static ClaySettings settings;

static void default_settings() {
    settings.fahrenheit = false;
    settings.fillCorners = false;
    settings.twoLineWeather = false;
    strcpy(settings.customKey, "");
    strcpy(settings.customLoc, "");
    strcpy(settings.colourScheme, "white");
}

static void save_settings() {
    int wrote = persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static void load_settings() {
    default_settings();
    int read = persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static void bluetooth_callback(bool connected) {
    layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_layer), connected);

    if (!connected) {
        vibes_double_pulse();
    }
}

static void outbox_iter() {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    dict_write_uint8(iter, 0, 0);

    app_message_outbox_send();
}

static void refresh_weather_layout() {
    layer_set_hidden(text_layer_get_layer(s_wea_layer), !settings.twoLineWeather);
    layer_set_hidden(text_layer_get_layer(s_tem_layer), !settings.twoLineWeather);
    layer_set_hidden(text_layer_get_layer(s_wea_tem_layer), settings.twoLineWeather);
}

static void update_background() {
    if (!s_background_layer)
        return;

    if (settings.fillCorners)
        bitmap_layer_set_bitmap(s_background_layer, s_background_filled_bitmap);
    else
        bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
}

static GColor text_colour() {
    if (strcmp(settings.colourScheme, "white") == 0 || strcmp(settings.colourScheme, "yellow") == 0)
        return GColorBlack;
    else
        return GColorWhite;
}

// i would have just used an ID for each colour scheme,
// but seemingly radio items have to have string values

static void update_colours() {
    gbitmap_destroy(s_background_filled_bitmap);
    gbitmap_destroy(s_background_bitmap);
    if (strcmp(settings.colourScheme, "white") == 0) {
        s_background_bitmap =
            gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUNDW);
        s_background_filled_bitmap =
            gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUNDW2);
    } else if (strcmp(settings.colourScheme, "blue") == 0) {
        s_background_bitmap =
            gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUNDB);
        s_background_filled_bitmap =
            gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUNDB2);
    } else if (strcmp(settings.colourScheme, "green") == 0) {
        s_background_bitmap =
            gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUNDG);
        s_background_filled_bitmap =
            gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUNDG2);
    } else if (strcmp(settings.colourScheme, "brown") == 0) {
        s_background_bitmap =
            gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUNDBR);
        s_background_filled_bitmap =
            gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUNDBR2);
    } else if (strcmp(settings.colourScheme, "black") == 0) {
        s_background_bitmap =
            gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUNDBL);
        s_background_filled_bitmap =
            gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUNDBL2);
    } else if (strcmp(settings.colourScheme, "red") == 0) {
        s_background_bitmap =
            gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUNDR);
        s_background_filled_bitmap =
            gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUNDR2);
    } else if (strcmp(settings.colourScheme, "yellow") == 0) {
        s_background_bitmap =
            gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUNDY);
        s_background_filled_bitmap =
            gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUNDY2);
    }
    text_layer_set_text_color(s_time_layer, text_colour());
    text_layer_set_text_color(s_wea_layer, text_colour());
    text_layer_set_text_color(s_dat_layer, text_colour());
    text_layer_set_text_color(s_tem_layer, text_colour());
    text_layer_set_text_color(s_wea_tem_layer, text_colour());
}

static void inbox_received_callback(DictionaryIterator *iterator,
                                    void *context) {
    static char temperature_buffer[8];
    static char conditions_buffer[32];
    static char weather_layer_buffer[32];

    Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_TEMPERATURE);
    Tuple *conditions_tuple = dict_find(iterator, MESSAGE_KEY_CONDITIONS);

    Tuple *fahrenheit_t = dict_find(iterator, MESSAGE_KEY_fahrenheit);
    if (fahrenheit_t) {
        settings.fahrenheit = fahrenheit_t->value->int32 == 1;
        persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
        outbox_iter();
    }

    Tuple *twoLineWeather_t = dict_find(iterator, MESSAGE_KEY_twoLineWeather);
    if (twoLineWeather_t) {
        settings.twoLineWeather = twoLineWeather_t->value->int32 == 1;
        save_settings();
        refresh_weather_layout();
    }

    Tuple *customKey_t = dict_find(iterator, MESSAGE_KEY_customKey);
    if (customKey_t) {
        strcpy(settings.customKey, customKey_t->value->cstring);
    }

    Tuple *customLoc_t = dict_find(iterator, MESSAGE_KEY_customLoc);
    if (customLoc_t) {
        strcpy(settings.customLoc, customLoc_t->value->cstring);
        printf("Loc was set to: %s", customLoc_t->value->cstring);
    }

    if (temp_tuple && conditions_tuple) {
        int temp = (int)temp_tuple->value->int32;
        if (settings.fahrenheit)
            temp = ((9 * temp) / 5) + 32;
        snprintf(temperature_buffer, sizeof(temperature_buffer), "%d°", temp);
        snprintf(conditions_buffer, sizeof(conditions_buffer), "%.7s", conditions_tuple->value->cstring);

        if (!settings.twoLineWeather) {
            if (strcmp(conditions_buffer, "Thunderstorm") == 0) {
                snprintf(conditions_buffer, sizeof(conditions_buffer), "%s",
                         "Thun’");
            } else if (strcmp(conditions_buffer, "Drizzle") == 0) {
                snprintf(conditions_buffer, sizeof(conditions_buffer), "%s",
                         "Driz’");
            } else if (strcmp(conditions_buffer, "Squall") == 0) {
                snprintf(conditions_buffer, sizeof(conditions_buffer), "%s",
                         "Squa’");
            } else if (strcmp(conditions_buffer, "Tornado") == 0) {
                snprintf(conditions_buffer, sizeof(conditions_buffer), "%s",
                         "Torn’");
            } else if (strcmp(conditions_buffer, "Clouds") == 0) {
                snprintf(conditions_buffer, sizeof(conditions_buffer), "%s",
                         "Cldy’");
            }
        }
        snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%d, %.7s",
                 temp, conditions_buffer);
        text_layer_set_text(s_wea_layer, conditions_buffer);
        text_layer_set_text(s_tem_layer, temperature_buffer);
        text_layer_set_text(s_wea_tem_layer, weather_layer_buffer);
    }

    Tuple *fillCorners_t = dict_find(iterator, MESSAGE_KEY_fillCorners);
    if (fillCorners_t) {
        settings.fillCorners = fillCorners_t->value->int32 == 1;
        update_background();
    }

    Tuple *colourScheme_t = dict_find(iterator, MESSAGE_KEY_colourScheme);
    if (colourScheme_t) {
        strcpy(settings.colourScheme, colourScheme_t->value->cstring);
        update_colours();
        update_background();
        save_settings();
    }

    refresh_weather_layout();
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator,
                                   AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void main_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    transport_med_small =
        fonts_load_custom_font(resource_get_handle(RESOURCE_ID_TRANSPORT_MED_18));
    transport_med =
        fonts_load_custom_font(resource_get_handle(RESOURCE_ID_TRANSPORT_MED_30));
    transport_he =
        fonts_load_custom_font(resource_get_handle(RESOURCE_ID_TRANSPORT_HEA_25));

    motorway =
        fonts_load_custom_font(resource_get_handle(RESOURCE_ID_MOTORWAY_33));

    s_background_bitmap =
        gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUNDW);
    s_background_filled_bitmap =
        gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUNDW2);

    s_bt_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT_ICON);

    s_bt_icon_layer = bitmap_layer_create(GRect(15, 63, 30, 30));

    s_background_layer = bitmap_layer_create(bounds);

    update_background();
    layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));
    bitmap_layer_set_bitmap(s_bt_icon_layer, s_bt_icon_bitmap);
    layer_add_child(window_layer, bitmap_layer_get_layer(s_bt_icon_layer));

    int time_x = 16;
    if (strcmp(settings.colourScheme, "brown") == 0)
        time_x = 20;
    s_time_layer = text_layer_create(GRect(time_x, 18, bounds.size.w, 50));
    text_layer_set_background_color(s_time_layer, GColorClear);
    text_layer_set_text_color(s_time_layer, text_colour());

    text_layer_set_font(s_time_layer, transport_med);
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

    s_bat_layer = text_layer_create(GRect(16, 113, bounds.size.w, 50));
    text_layer_set_background_color(s_bat_layer, GColorClear);
    text_layer_set_text_color(s_bat_layer, GColorWhite);
    text_layer_set_font(s_bat_layer, motorway);
    text_layer_set_text_alignment(s_bat_layer, GTextAlignmentCenter);

    s_dat_layer = text_layer_create(GRect(-32, 117, bounds.size.w, 50));
    text_layer_set_background_color(s_dat_layer, GColorClear);
    text_layer_set_text_color(s_dat_layer, text_colour());
    text_layer_set_font(s_dat_layer, transport_he);
    text_layer_set_text_alignment(s_dat_layer, GTextAlignmentCenter);

    s_wea_tem_layer = text_layer_create(GRect(47, 67, bounds.size.w, 50));
    text_layer_set_background_color(s_wea_tem_layer, GColorClear);
    text_layer_set_text_color(s_wea_tem_layer, text_colour());
    text_layer_set_font(s_wea_tem_layer, transport_med_small);
    text_layer_set_text_alignment(s_wea_tem_layer, GTextAlignmentLeft);
    layer_add_child(window_layer, text_layer_get_layer(s_wea_tem_layer));

    s_wea_layer = text_layer_create(GRect(7, 75, bounds.size.w, 50));
    text_layer_set_background_color(s_wea_layer, GColorClear);
    text_layer_set_text_color(s_wea_layer, text_colour());
    text_layer_set_font(s_wea_layer, transport_med_small);
    text_layer_set_text_alignment(s_wea_layer, GTextAlignmentCenter);

    s_tem_layer = text_layer_create(GRect(47, 58, bounds.size.w, 50));
    text_layer_set_background_color(s_tem_layer, GColorClear);
    text_layer_set_text_color(s_tem_layer, text_colour());
    text_layer_set_font(s_tem_layer, transport_med_small);
    text_layer_set_text_alignment(s_wea_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(s_wea_layer));
    layer_add_child(window_layer, text_layer_get_layer(s_tem_layer));
    layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
    layer_add_child(window_layer, text_layer_get_layer(s_bat_layer));
    layer_add_child(window_layer, text_layer_get_layer(s_dat_layer));
    int time_x = 16;
    if (strcmp(settings.colourScheme, "brown") == 0)
        time_x = 20;
    s_time_layer = text_layer_create(GRect(time_x, 18, bounds.size.w, 50));
    layer_set_hidden(text_layer_get_layer(s_wea_layer), !settings.twoLineWeather);
    layer_set_hidden(text_layer_get_layer(s_tem_layer), !settings.twoLineWeather);
    layer_set_hidden(text_layer_get_layer(s_wea_tem_layer), settings.twoLineWeather);
    bluetooth_callback(connection_service_peek_pebble_app_connection());
    update_colours();
}

static void main_window_unload(Window *window) {
    fonts_unload_custom_font(transport_med_small);
    fonts_unload_custom_font(transport_med);
    fonts_unload_custom_font(transport_he);
    gbitmap_destroy(s_bt_icon_bitmap);
    bitmap_layer_destroy(s_bt_icon_layer);
    gbitmap_destroy(s_background_filled_bitmap);
    gbitmap_destroy(s_background_bitmap);
    bitmap_layer_destroy(s_background_layer);
}

static void update_time() {
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    static char s_buffer[8];
    static char bat_buffer[5];
    static char dat_buffer[4];
    strftime(dat_buffer, sizeof(dat_buffer), "%a", tick_time);
    strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
    snprintf(bat_buffer, sizeof(bat_buffer), "%d", s_battery_level);
    text_layer_set_text(s_time_layer, s_buffer);
    text_layer_set_text(s_bat_layer, bat_buffer);
    text_layer_set_text(s_dat_layer, dat_buffer);
}

static void battery_callback(BatteryChargeState state) {
    s_battery_level = state.charge_percent;
    if (s_battery_level == 100)
        s_battery_level = 99;
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    if (tick_time->tm_min % 30 == 0)
        outbox_iter();
    update_time();
}

static void init() {
    load_settings();

    s_main_window = window_create();
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    battery_state_service_subscribe(battery_callback);
    battery_callback(battery_state_service_peek());

    app_message_register_inbox_received(inbox_received_callback);
    app_message_open(128, 128);
    app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_register_outbox_failed(outbox_failed_callback);
    app_message_register_outbox_sent(outbox_sent_callback);

    window_set_window_handlers(
        s_main_window,
        (WindowHandlers){.load = main_window_load, .unload = main_window_unload});

    window_stack_push(s_main_window, true);

    connection_service_subscribe((ConnectionHandlers){
        .pebble_app_connection_handler = bluetooth_callback});
}

static void deinit() { window_destroy(s_main_window); }

int main(void) {
    init();
    app_event_loop();
    deinit();
}