#include <pebble.h>
#include "date_layer.h"
#include "common.h"
#include "informer.h"


// types

struct DateLayer {
	Layer *back_layer;

	struct tm last_time;

	struct {
		WeatherInfo info;
		time_t info_time;
		bool is_waiting;
		AppTimer *waiting_timer;
		int16_t waiting_count;
	} weather;
};


const int16_t kWeatherWaitingStepsNumber	= 100;
const int16_t kWeatherActualTimeout			= 5 * 60; // 5 mins
const int16_t kWeatherHideTimeout			= 30 * 60; // 30 mins


// internal

static void request_weather_info(DateLayer *date_layer) {
	DictionaryIterator *iterator;
	app_message_outbox_begin(&iterator);
	dict_write_uint8(iterator, 0, 0);
	app_message_outbox_send();

	date_layer->weather.is_waiting = true;
	layer_mark_dirty(date_layer->back_layer);
}


// timers

static void handle_refresh_timer(void *data) {
	DateLayer *date_layer = data;

	date_layer->weather.waiting_timer = NULL;

	layer_mark_dirty(date_layer->back_layer);
}


// drawings

static void draw_background(DateLayer *date_layer, GContext *ctx) {
	const GRect bounds = layer_get_bounds(date_layer->back_layer);

	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_rect(ctx, bounds, 0, GCornerNone);
}


static void draw_day_month(DateLayer *date_layer, GContext *ctx) {
	const GRect bounds = layer_get_bounds(date_layer->back_layer);

	char text[0x20];
	strftime(text, sizeof(text), "%B %d", &date_layer->last_time);

	const GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
	const GTextOverflowMode mode = GTextOverflowModeWordWrap;
	const GTextAlignment align = GTextAlignmentCenter;

	GRect rect = bounds;
	rect.origin.y = 15;
	rect.size.h = graphics_text_layout_get_content_size(text, font, bounds, mode, align).h;

	graphics_context_set_text_color(ctx, GColorWhite);
	graphics_draw_text(ctx, text, font, rect, mode, align, NULL);
}


static void draw_weekday(DateLayer *date_layer, GContext *ctx) {
	const GRect bounds = layer_get_bounds(date_layer->back_layer);

	char text[0x20];
	strftime(text, sizeof(text), "%A", &date_layer->last_time);

	const GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
	const GTextOverflowMode mode = GTextOverflowModeWordWrap;
	const GTextAlignment align = GTextAlignmentCenter;

	GRect rect = bounds;
	rect.origin.y = 45;
	rect.size = graphics_text_layout_get_content_size(text, font, bounds, mode, align);
	rect.size.w = bounds.size.w;

	graphics_context_set_text_color(ctx, GColorWhite);
	graphics_draw_text(ctx, text, font, rect, mode, align, NULL);
}


static void draw_place(DateLayer *date_layer, GContext *ctx) {
	const GRect bounds = layer_get_bounds(date_layer->back_layer);

	const GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
	const GTextOverflowMode mode = GTextOverflowModeWordWrap;
	const GTextAlignment align = GTextAlignmentCenter;

	GRect rect = bounds;
	rect.origin.y = 88;
	rect.size.h = graphics_text_layout_get_content_size(date_layer->weather.info.place, font, bounds, mode, align).h;

	graphics_context_set_text_color(ctx, GColorBrass);
	graphics_draw_text(ctx, date_layer->weather.info.place, font, rect, mode, align, NULL);
}


static void draw_weather(DateLayer *date_layer, GContext *ctx) {
	const GRect bounds = layer_get_bounds(date_layer->back_layer);

	char deg_text[0x10];
	snprintf(deg_text, sizeof(deg_text), "%d°C", date_layer->weather.info.temperature);

	const GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
	const GTextOverflowMode mode = GTextOverflowModeWordWrap;
	const GTextAlignment align = GTextAlignmentLeft;

	GRect rect = bounds;
	rect.origin.x = 30;
	rect.origin.y = 110;
	rect.size = graphics_text_layout_get_content_size(deg_text, font, bounds, mode, align);

	graphics_context_set_text_color(ctx, GColorBrass);
	graphics_draw_text(ctx, deg_text, font, rect, mode, align, NULL);

	rect.origin.y += rect.size.h + 2;
	rect.size = graphics_text_layout_get_content_size(date_layer->weather.info.description, font, bounds, mode, align);

	graphics_draw_text(ctx, date_layer->weather.info.description, font, rect, mode, align, NULL);
}


static void draw_atmosphere(DateLayer *date_layer, GContext *ctx) {
	const GRect bounds = layer_get_bounds(date_layer->back_layer);

	char press_text[0x10], humid_text[0x10];
	snprintf(press_text, sizeof(press_text), "%dmm", date_layer->weather.info.pressure);
	snprintf(humid_text, sizeof(humid_text), "%d%%", date_layer->weather.info.humidity);

	const GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
	const GTextOverflowMode mode = GTextOverflowModeWordWrap;
	const GTextAlignment align = GTextAlignmentRight;

	GRect rect = bounds;
	rect.size.h = graphics_text_layout_get_content_size(press_text, font, bounds, mode, align).h;
	rect.origin.x = bounds.size.w - rect.size.w - 30;
	rect.origin.y = 110;

	graphics_context_set_text_color(ctx, GColorBrass);
	graphics_draw_text(ctx, press_text, font, rect, mode, align, NULL);

	rect.size.h = graphics_text_layout_get_content_size(humid_text, font, bounds, mode, align).h;
	rect.origin.y += rect.size.h + 2;

	graphics_draw_text(ctx, humid_text, font, rect, mode, align, NULL);
}


static void draw_waiting(DateLayer *date_layer, GContext *ctx) {
	const GRect bounds = layer_get_bounds(date_layer->back_layer);
	const GPoint center = grect_center_point(&bounds);

	const int16_t common_offset = TRIG_MAX_ANGLE * date_layer->weather.waiting_count / kWeatherWaitingStepsNumber;
	const int16_t angle_points = 2;
	const int16_t angle_step = TRIG_MAX_ANGLE / angle_points;

	for (int16_t i=0; i<angle_points; i++) {
		const int16_t angle = common_offset + (i * angle_step);
		const GPoint point = bk_second_point_for_rotation(center, center.y, angle);

		graphics_context_set_fill_color(ctx, GColorBrass);
		graphics_fill_circle(ctx, point, 1);
	}

	if (date_layer->weather.waiting_count++ > kWeatherWaitingStepsNumber) {
		date_layer->weather.waiting_count -= kWeatherWaitingStepsNumber;
	}
}


// services

static void handle_date_tick(struct tm *time, TimeUnits changed_units) {
	informer_inform_with_object(InformerEventDateTick, time);
}


// events

static void handle_date_tick_event(void *listener, void *object) {
	DateLayer *date_layer = listener;
	struct tm *time = object;

	date_layer->last_time = *time;

	layer_mark_dirty(date_layer->back_layer);
}


static void handle_weather_event(void *listener, void *object) {
	DateLayer *date_layer = listener;
	WeatherInfo *weather = object;

	strcpy(date_layer->weather.info.url, weather->url);
	strcpy(date_layer->weather.info.place, weather->place);
	date_layer->weather.info.temperature = weather->temperature;
	strcpy(date_layer->weather.info.description, weather->description);
	date_layer->weather.info.pressure = weather->pressure;
	date_layer->weather.info.humidity = weather->humidity;

	date_layer->weather.info_time = time(NULL);
	date_layer->weather.is_waiting = false;

	layer_mark_dirty(date_layer->back_layer);
}


// core

static void date_layer_draw(Layer *layer, GContext *ctx) {
	DateLayer *date_layer = layer_get_data(layer);

	draw_background(date_layer, ctx);
	draw_day_month(date_layer, ctx);
	draw_weekday(date_layer, ctx);

	if (time(NULL) - date_layer->weather.info_time < kWeatherHideTimeout) {
		draw_place(date_layer, ctx);
		draw_weather(date_layer, ctx);
		draw_atmosphere(date_layer, ctx);
	}

	if (date_layer->weather.is_waiting) {
		draw_waiting(date_layer, ctx);
		date_layer->weather.waiting_timer = app_timer_register(100, handle_refresh_timer, date_layer);
	}
}


DateLayer *date_layer_create(GRect rect) {
	Layer *layer = layer_create_with_data(rect, sizeof(DateLayer));
	layer_set_update_proc(layer, date_layer_draw);

	DateLayer *date_layer = layer_get_data(layer);
	date_layer->back_layer = layer;
	memset(&date_layer->weather, 0, sizeof(date_layer->weather));

	return date_layer;
}


Layer *date_layer_get_layer(DateLayer *date_layer) {
	return date_layer->back_layer;
}


void date_layer_got_focus(DateLayer *date_layer) {
	const time_t t = time(NULL);
	date_layer->last_time = *localtime(&t);
	layer_mark_dirty(date_layer->back_layer);

	informer_add_listener(InformerEventDateTick, date_layer, handle_date_tick_event);
	informer_add_listener(InformerEventWeather, date_layer, handle_weather_event);

	tick_timer_service_subscribe(DAY_UNIT, handle_date_tick);

	if (time(NULL) - date_layer->weather.info_time > kWeatherActualTimeout) {
		request_weather_info(date_layer);
	}
}


void date_layer_lost_focus(DateLayer *date_layer) {
	tick_timer_service_unsubscribe();

	informer_remove_listener(InformerEventDateTick, date_layer, handle_date_tick_event);
	informer_remove_listener(InformerEventWeather, date_layer, handle_weather_event);
}


void date_layer_destroy(DateLayer *date_layer) {
	if (date_layer->weather.waiting_timer) {
		app_timer_cancel(date_layer->weather.waiting_timer);
		date_layer->weather.waiting_timer = NULL;
	}

	layer_destroy(date_layer->back_layer);
}
