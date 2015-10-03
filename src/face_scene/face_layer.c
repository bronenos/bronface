#include <pebble.h>
#include "face_layer.h"
#include "common.h"
#include "informer.h"


// types

typedef enum {
	FaceLayerDashesModeNone,
	FaceLayerDashesModeMain,
	FaceLayerDashesModeAll,
	FaceLayerDashesMode_First	= FaceLayerDashesModeNone,
	FaceLayerDashesMode_Last	= FaceLayerDashesModeAll,
} FaceLayerDashesMode;

struct FaceLayer {
	Layer *back_layer;

	struct tm last_time;
	BatteryChargeState last_battery;

	FaceLayerDashesMode dashes_mode;
	bool show_seconds;
};


// config

const int16_t	kFaceLayerDashesCount		= 60;
const int16_t	kFaceLayerHoursCount		= 12;
const int16_t	kFaceLayerMinutesCount		= 60;
const int16_t	kFaceLayerSecondsCount		= 60;
const int16_t	kFaceLayerMainInterval		= 15;


// forward

static void subscribe_for_tick(FaceLayer *face_layer);
static void handle_time_tick(struct tm *time, TimeUnits changed_units);


// internal

static void face_layer_update_mode(FaceLayer *face_layer) {
	persist_write_int(PersistDataKeyWatchfaceMode, face_layer->dashes_mode);

	layer_mark_dirty(face_layer->back_layer);
}


static void face_layer_update_seconds_active(FaceLayer *face_layer) {
	persist_write_bool(PersistDataKeySecondsActive, face_layer->show_seconds);

	tick_timer_service_unsubscribe();
	subscribe_for_tick(face_layer);
}


static void subscribe_for_tick(FaceLayer *face_layer) {
	if (face_layer->show_seconds) {
		tick_timer_service_subscribe(SECOND_UNIT, handle_time_tick);
	}
	else {
		tick_timer_service_subscribe(MINUTE_UNIT, handle_time_tick);
	}
}


// drawing

static void draw_background(FaceLayer *face_layer, GContext *ctx) {
	const GRect bounds = layer_get_bounds(face_layer->back_layer);

	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_rect(ctx, bounds, 0, GCornerNone);
}


static void draw_dashes(FaceLayer *face_layer, GContext *ctx) {
	if (face_layer->dashes_mode == FaceLayerDashesModeNone) {
		return;
	}
	else {
		const GRect bounds = layer_get_bounds(face_layer->back_layer);
		const GPoint center = grect_center_point(&bounds);

		for (int16_t i=0; i<kFaceLayerDashesCount; i++) {
			const int16_t angle = TRIG_MAX_ANGLE * i / kFaceLayerDashesCount;

			const bool is_main_dash = (i % kFaceLayerMainInterval == 0);
			if (is_main_dash) {
				const GPoint point_from = bk_second_point_for_rotation(center, center.y, angle);
				const GPoint point_to = bk_second_point_for_rotation(center, center.y - 10, angle);
				const GPoint point_inner = bk_second_point_for_rotation(center, center.y - 6, angle);

				graphics_context_set_stroke_color(ctx, GColorLightGray);
				graphics_context_set_stroke_width(ctx, 9);
				graphics_draw_line(ctx, point_from, point_to);

				graphics_context_set_stroke_color(ctx, GColorBlack);
				graphics_context_set_stroke_width(ctx, 3);
				graphics_draw_line(ctx, point_inner, point_to);

				continue;
			}

			const bool is_hour_dash = (i % (kFaceLayerDashesCount / kFaceLayerHoursCount) == 0);
			if (is_hour_dash && (face_layer->dashes_mode > FaceLayerDashesModeNone)) {
				const GPoint point_from = bk_second_point_for_rotation(center, center.y, angle);
				const GPoint point_to = bk_second_point_for_rotation(center, center.y - 7, angle);

				graphics_context_set_stroke_color(ctx, GColorLightGray);
				graphics_context_set_stroke_width(ctx, 2);
				graphics_draw_line(ctx, point_from, point_to);

				continue;
			}

			const bool is_regular_dash = true;
			if (is_regular_dash && (face_layer->dashes_mode > FaceLayerDashesModeMain)) {
				const GPoint point = bk_second_point_for_rotation(center, center.y, angle);

				graphics_context_set_fill_color(ctx, GColorLightGray);
				graphics_fill_circle(ctx, point, 1);

				continue;
			}
		}
	}
}


static void draw_hands(FaceLayer *face_layer, GContext *ctx) {
	const GRect bounds = layer_get_bounds(face_layer->back_layer);
	const GPoint center = grect_center_point(&bounds);

	// minute
	const int16_t minute_angle = TRIG_MAX_ANGLE * face_layer->last_time.tm_min / kFaceLayerMinutesCount;
	const GPoint minute_outer_point = bk_second_point_for_rotation(center, 62, minute_angle);
	const GPoint minute_inner_point = bk_second_point_for_rotation(center, 14, minute_angle);

	graphics_context_set_stroke_color(ctx, GColorBrass);
	graphics_context_set_stroke_width(ctx, 1);
	graphics_draw_line(ctx, center, minute_outer_point);

	graphics_context_set_stroke_color(ctx, GColorBrass);
	graphics_context_set_stroke_width(ctx, 6);
	graphics_draw_line(ctx, minute_inner_point, minute_outer_point);

	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_context_set_stroke_width(ctx, 2);
	graphics_draw_line(ctx, minute_inner_point, minute_outer_point);

	// hour
	const int16_t hour_angle = TRIG_MAX_ANGLE * face_layer->last_time.tm_hour / kFaceLayerHoursCount;
	const GPoint hour_outer_point = bk_second_point_for_rotation(center, 45, hour_angle);
	const GPoint hour_inner_point = bk_second_point_for_rotation(center, 14, hour_angle);

	graphics_context_set_stroke_color(ctx, GColorBrass);
	graphics_context_set_stroke_width(ctx, 1);
	graphics_draw_line(ctx, center, hour_outer_point);

	graphics_context_set_stroke_color(ctx, GColorBrass);
	graphics_context_set_stroke_width(ctx, 5);
	graphics_draw_line(ctx, hour_inner_point, hour_outer_point);

	// second
	if (face_layer->show_seconds) {
		const int16_t second_angle = TRIG_MAX_ANGLE * face_layer->last_time.tm_sec / kFaceLayerSecondsCount;
		const GPoint second_point = bk_second_point_for_rotation(center, 72, second_angle);

		graphics_context_set_stroke_color(ctx, GColorChromeYellow);
		graphics_context_set_stroke_width(ctx, 1);
		graphics_draw_line(ctx, center, second_point);
	}
}


static void draw_center(FaceLayer *face_layer, GContext *ctx) {
	const GRect bounds = layer_get_bounds(face_layer->back_layer);
	const GPoint center = grect_center_point(&bounds);

	if (face_layer->last_battery.is_charging) {
		graphics_context_set_fill_color(ctx, GColorJaegerGreen);
	}
	else if (face_layer->last_battery.charge_percent <= 10) {
		graphics_context_set_fill_color(ctx, GColorFolly);
	}
	else {
		graphics_context_set_fill_color(ctx, GColorChromeYellow);
	}

	graphics_fill_circle(ctx, center, 4);
}


// services

static void handle_time_tick(struct tm *time, TimeUnits changed_units) {
	informer_inform_with_object(InformerEventTimeTick, time);
}


// events

static void handle_time_tick_event(void *listener, void *object) {
	FaceLayer *face_layer = listener;
	struct tm *time = object;

	face_layer->last_time = *time;
	layer_mark_dirty(face_layer->back_layer);
}


static void handle_tap_event(void *listener, void *object) {
	FaceLayer *face_layer = listener;

	if (face_layer->show_seconds) {
		face_layer->show_seconds = false;
		face_layer->dashes_mode = FaceLayerDashesModeMain;
	}
	else {
		face_layer->show_seconds = true;
		face_layer->dashes_mode = FaceLayerDashesModeAll;
	}

	face_layer_update_seconds_active(face_layer);
	face_layer_update_mode(face_layer);
}


static void handle_battery_event(void *listener, void *object) {
	FaceLayer *face_layer = listener;
	BatteryChargeState *battery = object;

	face_layer->last_battery = *battery;

	layer_mark_dirty(face_layer->back_layer);
}


// core

static void face_layer_draw(Layer *layer, GContext *ctx) {
	FaceLayer *face_layer = layer_get_data(layer);

	draw_background(face_layer, ctx);
	draw_dashes(face_layer, ctx);
	draw_hands(face_layer, ctx);
	draw_center(face_layer, ctx);
}


FaceLayer *face_layer_create(GRect rect) {
	Layer *layer = layer_create_with_data(rect, sizeof(FaceLayer));
	layer_set_update_proc(layer, face_layer_draw);

	FaceLayer *face_layer = layer_get_data(layer);
	face_layer->back_layer = layer;

	if (persist_exists(PersistDataKeyWatchfaceMode)) {
		face_layer->dashes_mode = persist_read_int(PersistDataKeyWatchfaceMode);
	}
	else {
		face_layer->dashes_mode = FaceLayerDashesModeAll;
	}

	if (persist_exists(PersistDataKeySecondsActive)) {
		face_layer->show_seconds = persist_read_bool(PersistDataKeySecondsActive);
	}
	else {
		face_layer->show_seconds = true;
	}

	face_layer->last_battery.is_charging = false;
	face_layer->last_battery.charge_percent = 100;

	return face_layer;
}


Layer *face_layer_get_layer(FaceLayer *face_layer) {
	return face_layer->back_layer;
}


void face_layer_got_focus(FaceLayer *face_layer) {
	const time_t t = time(NULL);
	face_layer->last_time = *localtime(&t);

	informer_add_listener(InformerEventTimeTick, face_layer, handle_time_tick_event);
	informer_add_listener(InformerEventAccelTap, face_layer, handle_tap_event);
	informer_add_listener(InformerEventBattery, face_layer, handle_battery_event);

	subscribe_for_tick(face_layer);

	layer_mark_dirty(face_layer->back_layer);
}


void face_layer_lost_focus(FaceLayer *face_layer) {
	tick_timer_service_unsubscribe();

	informer_remove_listener(InformerEventTimeTick, face_layer, handle_time_tick_event);
	informer_remove_listener(InformerEventAccelTap, face_layer, handle_tap_event);
	informer_remove_listener(InformerEventBattery, face_layer, handle_battery_event);
}


void face_layer_destroy(FaceLayer *face_layer) {
	layer_destroy(face_layer->back_layer);
}
