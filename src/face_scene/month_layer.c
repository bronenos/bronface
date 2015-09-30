#include <pebble.h>
#include "month_layer.h"
#include "common.h"
#include "informer.h"


// types

struct MonthLayer {
	Layer *back_layer;

	tm last_time;
};


// internal

static void draw_time_with_format(MonthLayer *month_layer, GContext *ctx, char *fmt, char *font_name, int16_t top_offset) {
	const GRect bounds = layer_get_bounds(month_layer->back_layer);

	char text[0xFF];
	strftime(text, sizeof(text), fmt, &month_layer->last_time);

	const GFont font = fonts_get_system_font(font_name);
	const GTextOverflowMode mode = GTextOverflowModeWordWrap;
	const GTextAlignment align = GTextAlignmentCenter;

	GRect drawing_rect;
	drawing_rect.size = graphics_text_layout_get_content_size(text, font, bounds, mode, align);
	drawing_rect.origin.x = (bounds.size.w - drawing_rect.size.w) / 2;
	drawing_rect.origin.y = (bounds.size.h - drawing_rect.size.h) / 2 + top_offset;

	graphics_draw_text(ctx, text, font, drawing_rect, mode, align, NULL);
}


// drawing

static void draw_background(MonthLayer *month_layer, GContext *ctx) {
	const GRect bounds = layer_get_bounds(month_layer->back_layer);

	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_rect(ctx, bounds, 0, GCornerNone);
}


static void draw_date(MonthLayer *month_layer, GContext *ctx) {
	graphics_context_set_text_color(ctx, GColorWhite);
	draw_time_with_format(month_layer, ctx, "%d", FONT_KEY_ROBOTO_BOLD_SUBSET_49, 0);
	draw_time_with_format(month_layer, ctx, "%A", FONT_KEY_GOTHIC_18, 40);
	draw_time_with_format(month_layer, ctx, "%B %Y", FONT_KEY_GOTHIC_24_BOLD, -50);
}


static void month_layer_update(Layer *layer, GContext *ctx) {
	MonthLayer *month_layer = layer_get_data(layer);

	draw_background(month_layer, ctx);
	draw_date(month_layer, ctx);
}


// events

static void handle_day_tick_event(void *listener, void *object) {
	MonthLayer *month_layer = listener;
	// tm *time = object;


	layer_mark_dirty(month_layer->back_layer);
}


// ticks

static void handle_day_tick(tm *time, TimeUnits changed_units) {
	informer_inform_with_object(InformerEventDayTick, time);
}


// core

MonthLayer *month_layer_create(GRect rect) {
	Layer *layer = layer_create_with_data(rect, sizeof(MonthLayer));
	layer_set_update_proc(layer, month_layer_update);

	MonthLayer *month_layer = layer_get_data(layer);
	month_layer->back_layer = layer;

	informer_add_listener(InformerEventDayTick, month_layer, handle_day_tick_event);

	return month_layer;
}


Layer *month_layer_get_layer(MonthLayer *month_layer) {
	return month_layer->back_layer;
}


void month_layer_did_get_focus(MonthLayer *month_layer) {
	const time_t t = time(NULL);
	month_layer->last_time = *localtime(&t);

	tick_timer_service_subscribe(DAY_UNIT, handle_day_tick);
}


void month_layer_did_lost_focus(MonthLayer *month_layer) {
	tick_timer_service_unsubscribe();
}


void month_layer_destroy(MonthLayer *month_layer) {
	informer_remove_listener(InformerEventDayTick, month_layer, handle_day_tick_event);

	layer_destroy(month_layer->back_layer);
}
