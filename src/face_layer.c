#include <pebble.h>
#include "face_layer.h"
#include "common.h"
#include "informer.h"


// types
enum FaceLayerDashesMode {
	FaceLayerDashesModeNone,
	FaceLayerDashesModeMain,
	FaceLayerDashesModeAll,
	FaceLayerDashesMode_First = FaceLayerDashesModeNone,
	FaceLayerDashesMode_Last = FaceLayerDashesModeAll,
};


struct FaceLayer {
	Layer *back_layer;

	GBitmap *dash_long_bitmap;
	GBitmap *dash_short_bitmap;
	GBitmap *hand_hour_bitmap;
	GBitmap *hand_minute_bitmap;

	bool has_time;
	struct tm last_time;

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

const GSize		kFaceLayerMainDashSize		= { .w = 5, .h = 15 };
const GSize		kFaceLayerHourDashSize		= { .w = 2, .h = 7 };
const GSize		kFaceLayerMinuteDashSize	= { .w = 1, .h = 5 };


// forward
static void subscribe_for_tick(struct FaceLayer *face_layer);
static void handle_minute_tick(struct tm *time, TimeUnits changed_units);


// internal
static void face_layer_update_mode(struct FaceLayer *face_layer) {
	persist_write_int(PersistDataKeyWatchfaceMode, face_layer->dashes_mode);

	layer_mark_dirty(face_layer->back_layer);
}


static void face_layer_update_seconds_active(struct FaceLayer *face_layer) {
	persist_write_bool(PersistDataKeySecondsActive, face_layer->seconds_active);

	tick_timer_service_unsubscribe();
	subscribe_for_tick(face_layer);
}


static void face_layer_get_bounds_and_center(struct FaceLayer *face_layer, GRect *bounds, GPoint *center) {
	*bounds = layer_get_bounds(face_layer->back_layer);

	if (center) {
		center->x = bounds->size.w * 0.5;
		center->y = bounds->size.h * 0.5;
	}
}


static GPoint face_layer_second_point_for_rotation(GPoint center, int16_t length, int16_t angle) {
	GPoint point;
	point.x = center.x + (length * sin_lookup(angle) / TRIG_MAX_RATIO);
	point.y = center.y + (length * -cos_lookup(angle) / TRIG_MAX_RATIO);
	return point;
}


static GPath *face_layer_path_for_two_points(GPoint first_point, GPoint second_point) {
	GPoint points[] = { first_point, second_point };

	static GPathInfo path_info;
	path_info.points = points;
	path_info.num_points = 2;

	return gpath_create(&path_info);
}


static void subscribe_for_tick(struct FaceLayer *face_layer) {
	if (face_layer->seconds_active) {
		tick_timer_service_subscribe(SECOND_UNIT, handle_minute_tick);
	}
	else {
		tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
	}
}


// drawing
static void draw_background(struct FaceLayer *face_layer, GContext *ctx) {
	GRect bounds;
	face_layer_get_bounds_and_center(face_layer, &bounds, NULL);

	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_rect(ctx, bounds, 0, GCornerNone);
}


static void draw_dashes(struct FaceLayer *face_layer, GContext *ctx) {
	if (face_layer->dashes_mode == FaceLayerDashesModeNone) {
		return;
	}
	else {
		GRect bounds;
		GPoint center;
		face_layer_get_bounds_and_center(face_layer, &bounds, &center);

		graphics_context_set_stroke_color(ctx, GColorWhite);

		for (int16_t i=0; i<kFaceLayerDashesCount; i++) {
			const int16_t angle = TRIG_MAX_ANGLE * i / kFaceLayerHoursCount;

			const GSize *dash_size = NULL;
			if (i % (kFaceLayerDashesCount / 4) == 0) {
				const bool dash_enabled = (face_layer->dashes_mode > FaceLayerDashesModeNone);
				dash_size = dash_enabled ? &kFaceLayerMainDashSize : NULL;
			}
			else if (i % (kFaceLayerDashesCount / kFaceLayerHoursCount) == 0) {
				const bool dash_enabled = (face_layer->dashes_mode > FaceLayerDashesModeNone);
				dash_size = dash_enabled ? &kFaceLayerHourDashSize : NULL;
			}
			else {
				const bool dash_enabled = (face_layer->dashes_mode > FaceLayerDashesModeMain);
				dash_size = dash_enabled ? &kFaceLayerMinuteDashSize : NULL;
			}

			const GPoint point_from = face_layer_second_point_for_rotation(center, center.y, angle);
			const GPoint point_to = face_layer_second_point_for_rotation(center, center.y - dash_size->h, angle);

			graphics_context_set_stroke_width(ctx, dash_size->w);
			graphics_draw_line(ctx, point_from, point_to);
		}
	}
}


static void draw_hands(struct FaceLayer *face_layer, GContext *ctx) {
	if (face_layer->has_time == false) {
		return;
	}

	GRect bounds;
	GPoint center;
	face_layer_get_bounds_and_center(face_layer, &bounds, &center);

	// second
	if (face_layer->seconds_active) {
		const int16_t second_angle = TRIG_MAX_ANGLE * face_layer->last_time.tm_sec / kFaceLayerSecondsCount;
		const GPoint second_point = face_layer_second_point_for_rotation(center, 80, second_angle);

		graphics_context_set_stroke_color(ctx, GColorWhite);
		graphics_context_set_stroke_width(ctx, 1);
		graphics_draw_line(ctx, center, second_point);
	}

	// minute
	const int16_t minute_angle = TRIG_MAX_ANGLE * face_layer->last_time.tm_min / kFaceLayerMinutesCount;
	const GPoint minute_long_point = face_layer_second_point_for_rotation(center, 65, minute_angle);
	const GPoint minute_short_point = face_layer_second_point_for_rotation(center, 64, minute_angle);

	graphics_context_set_stroke_color(ctx, GColorWhite);
	graphics_context_set_stroke_width(ctx, 7);
	graphics_draw_line(ctx, center, minute_long_point);

	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_context_set_stroke_width(ctx, 3);
	graphics_draw_line(ctx, center, minute_short_point);

	// hour
	const int16_t hour_angle = TRIG_MAX_ANGLE * face_layer->last_time.tm_hour / kFaceLayerHoursCount;
	const GPoint hour_point = face_layer_second_point_for_rotation(center, 40, hour_angle);

	graphics_context_set_stroke_color(ctx, GColorWhite);
	graphics_context_set_stroke_width(ctx, 5);
	graphics_draw_line(ctx, center, hour_point);
}


static void draw_center(struct FaceLayer *face_layer, GContext *ctx) {
	GRect bounds;
	GPoint center;
	face_layer_get_bounds_and_center(face_layer, &bounds, &center);

	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_circle(ctx, center, 4);
}


static void face_layer_update(Layer *layer, GContext *ctx) {
	struct FaceLayer *face_layer = (struct FaceLayer *) layer_get_data(layer);

	draw_background(face_layer, ctx);
	draw_dashes(face_layer, ctx);
	draw_hands(face_layer, ctx);
	draw_center(face_layer, ctx);
}


// informer handlers
static void handle_minute_timer(void *listener, void *object) {
	struct FaceLayer *face_layer = (struct FaceLayer *) listener;
	struct tm *time = (struct tm *) object;

	face_layer->last_time = *time;
	face_layer->has_time = true;

	layer_mark_dirty(face_layer->back_layer);
}


static void handle_up_click(void *listener, void *object) {
	struct FaceLayer *face_layer = (struct FaceLayer *) listener;

	face_layer->seconds_active = !face_layer->seconds_active;
	face_layer_update_seconds_active(face_layer);

}


static void handle_up_long_click(void *listener, void *object) {
	struct FaceLayer *face_layer = (struct FaceLayer *) listener;

	if (face_layer->dashes_mode-- == FaceLayerDashesMode_First) {
		face_layer->dashes_mode = FaceLayerDashesMode_Last;
	}

	face_layer_update_mode(face_layer);
}


static void handle_down_long_click(void *listener, void *object) {
	struct FaceLayer *face_layer = (struct FaceLayer *) listener;

	if (face_layer->dashes_mode++ == FaceLayerDashesMode_Last) {
		face_layer->dashes_mode = FaceLayerDashesMode_First;
	}

	face_layer_update_mode(face_layer);
}


static void handle_select_click(void *listener, void *object) {
	// todo: show the day info
}


// tick handler
static void handle_minute_tick(struct tm *time, TimeUnits changed_units) {
	informer_inform_with_object(InformerEventMinuteTimer, time);
}


// core
struct FaceLayer *face_layer_create(GRect rect) {
	Layer *layer = layer_create_with_data(rect, sizeof(struct FaceLayer));
	layer_set_update_proc(layer, face_layer_update);

	struct FaceLayer *face_layer = (struct FaceLayer *) layer_get_data(layer);
	face_layer->back_layer = layer;
	face_layer->dash_long_bitmap = gbitmap_create_with_resource(RESOURCE_ID_DASH_LONG);
	face_layer->dash_short_bitmap = gbitmap_create_with_resource(RESOURCE_ID_DASH_SHORT);
	face_layer->hand_hour_bitmap = gbitmap_create_with_resource(RESOURCE_ID_HAND_HOUR);
	face_layer->hand_minute_bitmap = gbitmap_create_with_resource(RESOURCE_ID_HAND_MINUTE);
	face_layer->has_time = false;
	face_layer->dashes_mode = FaceLayerDashesModeAll;
	face_layer->seconds_active = false;

	if (persist_exists(PersistDataKeyWatchfaceMode)) {
		face_layer->dashes_mode = persist_read_int(PersistDataKeyWatchfaceMode);
	}

	if (persist_exists(PersistDataKeySecondsActive)) {
		face_layer->seconds_active = persist_read_bool(PersistDataKeySecondsActive);
	}

	informer_add_listener(InformerEventMinuteTimer, face_layer, handle_minute_timer);
	informer_add_listener(InformerEventUpClick, face_layer, handle_up_click);
	informer_add_listener(InformerEventUpLongClick, face_layer, handle_up_long_click);
	informer_add_listener(InformerEventDownLongClick, face_layer, handle_down_long_click);
	informer_add_listener(InformerEventSelectClick, face_layer, handle_select_click);

	tick_timer_service_subscribe(SECOND_UNIT, handle_minute_tick);

	return face_layer;
}


Layer *face_layer_get_layer(struct FaceLayer *face_layer) {
	return face_layer->back_layer;
}


void face_layer_destroy(struct FaceLayer *face_layer) {
	tick_timer_service_unsubscribe();

	informer_remove_listener(InformerEventMinuteTimer, face_layer, handle_minute_timer);
	informer_remove_listener(InformerEventSelectClick, face_layer, handle_up_click);
	informer_remove_listener(InformerEventSelectClick, face_layer, handle_up_long_click);
	informer_remove_listener(InformerEventSelectClick, face_layer, handle_down_long_click);
	informer_remove_listener(InformerEventSelectClick, face_layer, handle_select_click);

	layer_destroy(face_layer->back_layer);

	gbitmap_destroy(face_layer->dash_long_bitmap);
	gbitmap_destroy(face_layer->dash_short_bitmap);
	gbitmap_destroy(face_layer->hand_hour_bitmap);
	gbitmap_destroy(face_layer->hand_minute_bitmap);

	free(face_layer);
}
