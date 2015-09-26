#include <pebble.h>
#include "face_layer.h"


const int kFaceLayerHoursCount		= 12;
const int kFaceLayerMinutesCount	= 60;


typedef enum {
	FaceLayerAnchorModeDashLong,
	FaceLayerAnchorModeDashShort,
	FaceLayerAnchorModeHandHour,
	FaceLayerAnchorModeHandMinute,
} FaceLayerAnchorMode;


static GPoint ui_face_layer_anchor_point(GBitmap *bitmap, FaceLayerAnchorMode mode) {
	GPoint point = GPointZero;
	const GSize bitmap_size = gbitmap_get_bounds(bitmap).size;

	switch (mode) {
	case FaceLayerAnchorModeDashLong:
	case FaceLayerAnchorModeDashShort:
		point.x = bitmap_size.w / 2;
		point.y = 0;
		break;

	case FaceLayerAnchorModeHandHour:
	case FaceLayerAnchorModeHandMinute:
		point.x = bitmap_size.w / 2;
		point.y = bitmap_size.h;
		break;
	}

	return point;
}


static void ui_face_layer_update(Layer *layer, GContext *ctx) {
	FaceLayer *face_layer = (FaceLayer *) layer_get_data(layer);
	const GRect layer_bounds = layer_get_bounds(layer);

	// fill with black color
	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_rect(ctx, layer_bounds, 0, GCornerNone);

	// draw hours
	graphics_context_set_fill_color(ctx, GColorWhite);

	GPoint center_point, anchor_point;
	center_point.x = layer_bounds.size.w / 2;
	center_point.y = layer_bounds.size.h / 2;

	for (int16_t i=0; i<kFaceLayerHoursCount; i++) {
		const int16_t angle = TRIG_MAX_ANGLE * i / kFaceLayerHoursCount;
		const bool is_third_hour = (i % 3 == 0);

		GPoint round_point;
		round_point.x = center_point.x + (center_point.x * sin_lookup(angle) / TRIG_MAX_RATIO);
		round_point.y = center_point.y + (center_point.y * -cos_lookup(angle) / TRIG_MAX_RATIO);

		if (is_third_hour) {
			anchor_point = ui_face_layer_anchor_point(face_layer->dash_long_bitmap, FaceLayerAnchorModeDashLong);
			graphics_draw_rotated_bitmap(ctx, face_layer->dash_long_bitmap, anchor_point, angle, round_point);
		}
		else {
			anchor_point = ui_face_layer_anchor_point(face_layer->dash_short_bitmap, FaceLayerAnchorModeDashShort);
			graphics_draw_rotated_bitmap(ctx, face_layer->dash_short_bitmap, anchor_point, angle, round_point);
		}
	}

	const int16_t hour_angle = TRIG_MAX_ANGLE * face_layer->last_tm.tm_hour / kFaceLayerHoursCount;
	anchor_point = ui_face_layer_anchor_point(face_layer->hand_hour_bitmap, FaceLayerAnchorModeHandHour);
	graphics_draw_rotated_bitmap(ctx, face_layer->hand_hour_bitmap, anchor_point, hour_angle, center_point);

	const int16_t minute_angle = TRIG_MAX_ANGLE * face_layer->last_tm.tm_min / kFaceLayerMinutesCount;
	anchor_point = ui_face_layer_anchor_point(face_layer->hand_minute_bitmap, FaceLayerAnchorModeHandMinute);
	graphics_draw_rotated_bitmap(ctx, face_layer->hand_minute_bitmap, anchor_point, minute_angle, center_point);
}


static FaceLayer *g_face_layer;
static void handle_minute_tick(struct tm *time, TimeUnits changed_units) {
	g_face_layer->last_tm = *time;
	layer_mark_dirty(g_face_layer->back_layer);
}


FaceLayer *ui_face_layer_create(GRect rect) {
	Layer *layer = layer_create_with_data(rect, sizeof(FaceLayer));
	layer_set_update_proc(layer, ui_face_layer_update);

	FaceLayer *face_layer = (FaceLayer *) layer_get_data(layer);
	face_layer->back_layer = layer;
	face_layer->dash_long_bitmap = gbitmap_create_with_resource(RESOURCE_ID_DASH_LONG);
	face_layer->dash_short_bitmap = gbitmap_create_with_resource(RESOURCE_ID_DASH_SHORT);
	face_layer->hand_hour_bitmap = gbitmap_create_with_resource(RESOURCE_ID_HAND_HOUR);
	face_layer->hand_minute_bitmap = gbitmap_create_with_resource(RESOURCE_ID_HAND_MINUTE);

	g_face_layer = face_layer;
	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);

	return face_layer;
}


void ui_face_layer_destroy(FaceLayer *face_layer) {
	tick_timer_service_unsubscribe();
	layer_destroy(face_layer->back_layer);
	gbitmap_destroy(face_layer->dash_long_bitmap);
	gbitmap_destroy(face_layer->dash_short_bitmap);
	gbitmap_destroy(face_layer->hand_hour_bitmap);
	gbitmap_destroy(face_layer->hand_minute_bitmap);
	free(face_layer);
}
