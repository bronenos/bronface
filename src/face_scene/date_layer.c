#include <pebble.h>
#include "date_layer.h"
#include "common.h"
#include "informer.h"


// types

struct WeatherInfo {
	int16_t degrees;
	char short_info[0x20];
	int16_t pressure;
	int16_t humidity;
};
typedef struct WeatherInfo WeatherInfo;


struct DateLayer {
	Layer *back_layer;

	tm last_time;
	WeatherInfo *weather_info;
};


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
	rect.size = graphics_text_layout_get_content_size(text, font, bounds, mode, align);
	rect.size.w = bounds.size.w;

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


static void draw_weather(DateLayer *date_layer, GContext *ctx) {
	if (date_layer->weather_info == NULL) {
		return;
	}

	const GRect bounds = layer_get_bounds(date_layer->back_layer);

	char deg_text[0x10], sky_text[0x20];
	snprintf(deg_text, sizeof(deg_text), "%d°", date_layer->weather_info->degrees);
	snprintf(sky_text, sizeof(sky_text), "%s", date_layer->weather_info->short_info);

	const GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
	const GTextOverflowMode mode = GTextOverflowModeWordWrap;
	const GTextAlignment align = GTextAlignmentLeft;

	GRect rect = bounds;
	rect.origin.x = 20;
	rect.origin.y = 90;
	rect.size = graphics_text_layout_get_content_size(deg_text, font, bounds, mode, align);

	graphics_context_set_text_color(ctx, GColorWhite);
	graphics_draw_text(ctx, deg_text, font, rect, mode, align, NULL);

	rect.origin.y += rect.size.h + 2;
	rect.size = graphics_text_layout_get_content_size(sky_text, font, bounds, mode, align);

	graphics_draw_text(ctx, sky_text, font, rect, mode, align, NULL);
}


static void draw_atmosphere(DateLayer *date_layer, GContext *ctx) {
	if (date_layer->weather_info == NULL) {
		return;
	}
	
	const GRect bounds = layer_get_bounds(date_layer->back_layer);

	char press_text[0x10], humid_text[0x10];
	snprintf(press_text, sizeof(press_text), "%dmm", date_layer->weather_info->pressure);
	snprintf(humid_text, sizeof(humid_text), "%d%%", date_layer->weather_info->humidity);

	const GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
	const GTextOverflowMode mode = GTextOverflowModeWordWrap;
	const GTextAlignment align = GTextAlignmentRight;

	GRect rect = bounds;
	rect.size.h = graphics_text_layout_get_content_size(press_text, font, bounds, mode, align).h;
	rect.origin.x = bounds.size.w - rect.size.w - 20;
	rect.origin.y = 90;

	graphics_context_set_text_color(ctx, GColorWhite);
	graphics_draw_text(ctx, press_text, font, rect, mode, align, NULL);

	rect.size.h = graphics_text_layout_get_content_size(humid_text, font, bounds, mode, align).h;
	rect.origin.y += rect.size.h + 2;

	graphics_draw_text(ctx, humid_text, font, rect, mode, align, NULL);
}


// services

static void handle_date_tick(tm *time, TimeUnits changed_units) {
	informer_inform_with_object(InformerEventTimeTick, time);
}


// events

static void handle_date_tick_event(void *listener, void *object) {
	DateLayer *date_layer = listener;
	tm *time = object;

	date_layer->last_time = *time;
	layer_mark_dirty(date_layer->back_layer);
}


// core

static void date_layer_draw(Layer *layer, GContext *ctx) {
	DateLayer *date_layer = layer_get_data(layer);

	draw_background(date_layer, ctx);
	draw_day_month(date_layer, ctx);
	draw_weekday(date_layer, ctx);
	draw_weather(date_layer, ctx);
	draw_atmosphere(date_layer, ctx);
}


DateLayer *date_layer_create(GRect rect) {
	Layer *layer = layer_create_with_data(rect, sizeof(DateLayer));
	layer_set_update_proc(layer, date_layer_draw);

	DateLayer *date_layer = layer_get_data(layer);
	date_layer->back_layer = layer;
	date_layer->weather_info = NULL;

	return date_layer;
}


Layer *date_layer_get_layer(DateLayer *date_layer) {
	return date_layer->back_layer;
}


void date_layer_got_focus(DateLayer *date_layer) {
	const time_t t = time(NULL);
	date_layer->last_time = *localtime(&t);
	layer_mark_dirty(date_layer->back_layer);

	informer_add_listener(InformerEventTimeTick, date_layer, handle_date_tick_event);

	tick_timer_service_subscribe(DAY_UNIT, handle_date_tick);
}


void date_layer_lost_focus(DateLayer *date_layer) {
	tick_timer_service_unsubscribe();

	informer_remove_listener(InformerEventTimeTick, date_layer, handle_date_tick_event);
}


void date_layer_destroy(DateLayer *date_layer) {
	if (date_layer->weather_info) {
		free(date_layer->weather_info);
		date_layer->weather_info = NULL;
	}

	layer_destroy(date_layer->back_layer);
}
