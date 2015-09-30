#include <pebble.h>
#include "face_layer.h"
#include "common.h"
#include "informer.h"


// types

enum FaceLayerDashesMode {
	FaceLayerDashesModeNone,
	FaceLayerDashesModeMain,
	FaceLayerDashesModeAll,
	FaceLayerDashesMode_First	= FaceLayerDashesModeNone,
	FaceLayerDashesMode_Last	= FaceLayerDashesModeAll,
};


struct FaceLayer {
	Layer *back_layer;

	tm last_time;
	enum FaceLayerDashesMode dashes_mode;
	bool seconds_active;
};


typedef enum {
	FaceLayerAnchorModeDashLong,
	FaceLayerAnchorModeDashShort,
	FaceLayerAnchorModeHandHour,
	FaceLayerAnchorModeHandMinute,
} FaceLayerAnchorMode;


// config

const int16_t	kFaceLayerDashesCount		= 60;
const int16_t	kFaceLayerHoursCount		= 12;
const int16_t	kFaceLayerMinutesCount		= 60;
const int16_t	kFaceLayerSecondsCount		= 60;
const int16_t	kFaceLayerMainInterval		= 15;


// forward

static void subscribe_for_tick(FaceLayer *face_layer);
static void handle_time_tick(tm *time, TimeUnits changed_units);


// internal

static void face_layer_update_mode(FaceLayer *face_layer) {
	persist_write_int(PersistDataKeyWatchfaceMode, face_layer->dashes_mode);

	layer_mark_dirty(face_layer->back_layer);
}


static void face_layer_update_seconds_active(FaceLayer *face_layer) {
	persist_write_bool(PersistDataKeySecondsActive, face_layer->seconds_active);

	tick_timer_service_unsubscribe();
	subscribe_for_tick(face_layer);
}


static GPoint face_layer_second_point_for_rotation(GPoint center, int16_t length, int16_t angle) {
	GPoint point;
	point.x = center.x + (length * sin_lookup(angle) / TRIG_MAX_RATIO);
	point.y = center.y + (length * -cos_lookup(angle) / TRIG_MAX_RATIO);
	return point;
}


static void subscribe_for_tick(FaceLayer *face_layer) {
	if (face_layer->seconds_active) {
		tick_timer_service_subscribe(SECOND_UNIT, handle_time_tick);
	}
	else {
		tick_timer_service_subscribe(MINUTE_UNIT, handle_time_tick);
	}
}


static GColor color_for_background() {
	GColor colors[DeviceColor_Count];
	colors[DeviceColorSilver]	= GColorWhite;
	colors[DeviceColorBlack]	= GColorBlack;
	colors[DeviceColorRose]		= GColorMelon;
	return colors[bk_get_device_color()];
}


static GColor color_for_main_dash() {
	GColor colors[DeviceColor_Count];
	colors[DeviceColorSilver]	= GColorWhite;
	colors[DeviceColorBlack]	= GColorLightGray;
	colors[DeviceColorRose]		= GColorMelon;
	return colors[bk_get_device_color()];
}


static GColor color_for_regular_dash() {
	GColor colors[DeviceColor_Count];
	colors[DeviceColorSilver]	= GColorWhite;
	colors[DeviceColorBlack]	= GColorLightGray;
	colors[DeviceColorRose]		= GColorMelon;
	return colors[bk_get_device_color()];
}


static GColor color_for_hour_hand() {
	GColor colors[DeviceColor_Count];
	colors[DeviceColorSilver]	= GColorWhite;
	colors[DeviceColorBlack]	= GColorBrass;
	colors[DeviceColorRose]		= GColorMelon;
	return colors[bk_get_device_color()];
}


static GColor color_for_minute_hand() {
	GColor colors[DeviceColor_Count];
	colors[DeviceColorSilver]	= GColorWhite;
	colors[DeviceColorBlack]	= GColorBrass;
	colors[DeviceColorRose]		= GColorMelon;
	return colors[bk_get_device_color()];
}


static GColor color_for_second_hand() {
	GColor colors[DeviceColor_Count];
	colors[DeviceColorSilver]	= GColorWhite;
	colors[DeviceColorBlack]	= GColorChromeYellow;
	colors[DeviceColorRose]		= GColorMelon;
	return colors[bk_get_device_color()];
}


// drawing

static void draw_background(FaceLayer *face_layer, GContext *ctx) {
	const GRect bounds = layer_get_bounds(face_layer->back_layer);

	graphics_context_set_fill_color(ctx, color_for_background());
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
			if (is_main_dash && (face_layer->dashes_mode > FaceLayerDashesModeNone)) {
				const GPoint point_from = face_layer_second_point_for_rotation(center, center.y, angle);
				const GPoint point_to = face_layer_second_point_for_rotation(center, center.y - 10, angle);
				const GPoint point_inner = face_layer_second_point_for_rotation(center, center.y - 6, angle);

				graphics_context_set_stroke_color(ctx, color_for_main_dash());
				graphics_context_set_stroke_width(ctx, 7);
				graphics_draw_line(ctx, point_from, point_to);

				graphics_context_set_stroke_color(ctx, color_for_background());
				graphics_context_set_stroke_width(ctx, 3);
				graphics_draw_line(ctx, point_inner, point_to);

				continue;
			}

			const bool is_hour_dash = (i % (kFaceLayerDashesCount / kFaceLayerHoursCount) == 0);
			if (is_hour_dash && (face_layer->dashes_mode > FaceLayerDashesModeNone)) {
				const GPoint point_from = face_layer_second_point_for_rotation(center, center.y, angle);
				const GPoint point_to = face_layer_second_point_for_rotation(center, center.y - 7, angle);

				graphics_context_set_stroke_color(ctx, color_for_regular_dash());
				graphics_context_set_stroke_width(ctx, 2);
				graphics_draw_line(ctx, point_from, point_to);

				continue;
			}

			const bool is_regular_dash = true;
			if (is_regular_dash && (face_layer->dashes_mode > FaceLayerDashesModeMain)) {
				const GPoint point = face_layer_second_point_for_rotation(center, center.y, angle);

				graphics_context_set_stroke_color(ctx, color_for_regular_dash());
				graphics_context_set_stroke_width(ctx, 1);
				graphics_draw_pixel(ctx, point);

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
	const GPoint minute_outer_point = face_layer_second_point_for_rotation(center, 62, minute_angle);
	const GPoint minute_inner_point = face_layer_second_point_for_rotation(center, 14, minute_angle);

	graphics_context_set_stroke_color(ctx, color_for_minute_hand());
	graphics_context_set_stroke_width(ctx, 1);
	graphics_draw_line(ctx, center, minute_outer_point);

	graphics_context_set_stroke_color(ctx, color_for_minute_hand());
	graphics_context_set_stroke_width(ctx, 6);
	graphics_draw_line(ctx, minute_inner_point, minute_outer_point);

	graphics_context_set_stroke_color(ctx, color_for_background());
	graphics_context_set_stroke_width(ctx, 2);
	graphics_draw_line(ctx, minute_inner_point, minute_outer_point);

	// hour
	const int16_t hour_angle = TRIG_MAX_ANGLE * face_layer->last_time.tm_hour / kFaceLayerHoursCount;
	const GPoint hour_outer_point = face_layer_second_point_for_rotation(center, 45, hour_angle);
	const GPoint hour_inner_point = face_layer_second_point_for_rotation(center, 14, hour_angle);

	graphics_context_set_stroke_color(ctx, color_for_hour_hand());
	graphics_context_set_stroke_width(ctx, 1);
	graphics_draw_line(ctx, center, hour_outer_point);

	graphics_context_set_stroke_color(ctx, color_for_hour_hand());
	graphics_context_set_stroke_width(ctx, 5);
	graphics_draw_line(ctx, hour_inner_point, hour_outer_point);

	// second
	if (face_layer->seconds_active) {
		const int16_t second_angle = TRIG_MAX_ANGLE * face_layer->last_time.tm_sec / kFaceLayerSecondsCount;
		const GPoint second_point = face_layer_second_point_for_rotation(center, 72, second_angle);

		graphics_context_set_stroke_color(ctx, color_for_second_hand());
		graphics_context_set_stroke_width(ctx, 1);
		graphics_draw_line(ctx, center, second_point);
	}
}


static void draw_center(FaceLayer *face_layer, GContext *ctx) {
	const GRect bounds = layer_get_bounds(face_layer->back_layer);
	const GPoint center = grect_center_point(&bounds);

	graphics_context_set_fill_color(ctx, color_for_second_hand());
	graphics_fill_circle(ctx, center, 4);
}


static void face_layer_update(Layer *layer, GContext *ctx) {
	FaceLayer *face_layer = layer_get_data(layer);

	draw_background(face_layer, ctx);
	draw_dashes(face_layer, ctx);
	draw_hands(face_layer, ctx);
	draw_center(face_layer, ctx);
}


// events

static void handle_time_tick_event(void *listener, void *object) {
	FaceLayer *face_layer = listener;
	tm *time = object;

	face_layer->last_time = *time;

	layer_mark_dirty(face_layer->back_layer);
}


static void handle_accel_event(void *listener, void *object) {
	FaceLayer *face_layer = listener;
	AccelAxisType *axis = (AccelAxisType *) object;

	if (*axis == ACCEL_AXIS_Y) {
		if (face_layer->dashes_mode++ == FaceLayerDashesMode_Last) {
			face_layer->dashes_mode = FaceLayerDashesMode_First;
		}

		face_layer_update_mode(face_layer);
	}
	else if (*axis == ACCEL_AXIS_X) {
		face_layer->seconds_active = !face_layer->seconds_active;
		face_layer_update_seconds_active(face_layer);
	}
}


// ticks

static void handle_time_tick(tm *time, TimeUnits changed_units) {
	informer_inform_with_object(InformerEventTimeTick, time);
}


// core

FaceLayer *face_layer_create(GRect rect) {
	Layer *layer = layer_create_with_data(rect, sizeof(FaceLayer));
	layer_set_update_proc(layer, face_layer_update);

	FaceLayer *face_layer = layer_get_data(layer);
	face_layer->back_layer = layer;

	if (persist_exists(PersistDataKeyWatchfaceMode)) {
		face_layer->dashes_mode = persist_read_int(PersistDataKeyWatchfaceMode);
	}
	else {
		face_layer->dashes_mode = FaceLayerDashesModeAll;
	}

	if (persist_exists(PersistDataKeySecondsActive)) {
		face_layer->seconds_active = persist_read_bool(PersistDataKeySecondsActive);
	}
	else {
		face_layer->seconds_active = true;
	}

	informer_add_listener(InformerEventTimeTick, face_layer, handle_time_tick_event);
	informer_add_listener(InformerEventAccel, face_layer, handle_accel_event);

	return face_layer;
}


Layer *face_layer_get_layer(FaceLayer *face_layer) {
	return face_layer->back_layer;
}


void face_layer_did_get_focus(FaceLayer *face_layer) {
	const time_t t = time(NULL);
	face_layer->last_time = *localtime(&t);

	subscribe_for_tick(face_layer);

	layer_mark_dirty(face_layer->back_layer);
}


void face_layer_did_lost_focus(FaceLayer *face_layer) {
	tick_timer_service_unsubscribe();
}


void face_layer_destroy(FaceLayer *face_layer) {
	informer_remove_listener(InformerEventTimeTick, face_layer, handle_time_tick_event);
	informer_remove_listener(InformerEventAccel, face_layer, handle_accel_event);

	layer_destroy(face_layer->back_layer);
}
