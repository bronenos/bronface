#include <pebble.h>
#include "face_layer.h"
#include "common.h"
#include "informer.h"


enum FaceLayerMode {
	FaceLayerModeHandsOnly,
	FaceLayerModeHandsWithMainDashes,
	FaceLayerModeHandsWithAllDashes,
};


struct FaceLayer {
	Layer *back_layer;

	GBitmap *dash_long_bitmap;
	GBitmap *dash_short_bitmap;
	GBitmap *hand_hour_bitmap;
	GBitmap *hand_minute_bitmap;

	bool has_time;
	struct tm last_time;

	enum FaceLayerMode mode;
};


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


static void draw_background(struct FaceLayer *face_layer, GContext *ctx) {
	const GRect layer_bounds = layer_get_bounds(face_layer->back_layer);

	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_rect(ctx, layer_bounds, 0, GCornerNone);
}


static void draw_hands(struct FaceLayer *face_layer, GContext *ctx) {
	if (face_layer->has_time == false) {
		return;
	}

	const GRect layer_bounds = layer_get_bounds(face_layer->back_layer);

	GPoint center_point, anchor_point;
	center_point.x = layer_bounds.size.w / 2;
	center_point.y = layer_bounds.size.h / 2;

	const int16_t hour_angle = TRIG_MAX_ANGLE * face_layer->last_time.tm_hour / kFaceLayerHoursCount;
	anchor_point = ui_face_layer_anchor_point(face_layer->hand_hour_bitmap, FaceLayerAnchorModeHandHour);
	graphics_draw_rotated_bitmap(ctx, face_layer->hand_hour_bitmap, anchor_point, hour_angle, center_point);

	const int16_t minute_angle = TRIG_MAX_ANGLE * face_layer->last_time.tm_min / kFaceLayerMinutesCount;
	anchor_point = ui_face_layer_anchor_point(face_layer->hand_minute_bitmap, FaceLayerAnchorModeHandMinute);
	graphics_draw_rotated_bitmap(ctx, face_layer->hand_minute_bitmap, anchor_point, minute_angle, center_point);
}


static void draw_dashes_all(struct FaceLayer *face_layer, GContext *ctx, bool all) {
	const GRect layer_bounds = layer_get_bounds(face_layer->back_layer);

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
		else if (all) {
			anchor_point = ui_face_layer_anchor_point(face_layer->dash_short_bitmap, FaceLayerAnchorModeDashShort);
			graphics_draw_rotated_bitmap(ctx, face_layer->dash_short_bitmap, anchor_point, angle, round_point);
		}
	}
}


static void ui_face_layer_update(Layer *layer, GContext *ctx) {
	struct FaceLayer *face_layer = (struct FaceLayer *) layer_get_data(layer);

	draw_background(face_layer, ctx);

	switch (face_layer->mode) {
	case FaceLayerModeHandsOnly:
		draw_hands(face_layer, ctx);
		break;

	case FaceLayerModeHandsWithMainDashes:
		draw_hands(face_layer, ctx);
		draw_dashes_all(face_layer, ctx, false);
		break;
		
	case FaceLayerModeHandsWithAllDashes:
		draw_hands(face_layer, ctx);
		draw_dashes_all(face_layer, ctx, true);
		break;
	}
}


static void handle_minute_event(void *listener, void *object) {
	struct FaceLayer *face_layer = (struct FaceLayer *) listener;
	struct tm *time = (struct tm *) object;

	face_layer->last_time = *time;
	face_layer->has_time = true;

	layer_mark_dirty(face_layer->back_layer);
}


static void handle_select_event(void *listener, void *object) {
	struct FaceLayer *face_layer = (struct FaceLayer *) listener;

	switch (face_layer->mode) {
	case FaceLayerModeHandsOnly:
		face_layer->mode = FaceLayerModeHandsWithMainDashes;
		break;

	case FaceLayerModeHandsWithMainDashes:
		face_layer->mode = FaceLayerModeHandsWithAllDashes;
		break;

	case FaceLayerModeHandsWithAllDashes:
		face_layer->mode = FaceLayerModeHandsOnly;
		break;
	}

	persist_write_int(PersistDataKeyWatchfaceMode, face_layer->mode);

	layer_mark_dirty(face_layer->back_layer);
}


struct FaceLayer *ui_face_layer_create(GRect rect) {
	Layer *layer = layer_create_with_data(rect, sizeof(struct FaceLayer));
	layer_set_update_proc(layer, ui_face_layer_update);

	struct FaceLayer *face_layer = (struct FaceLayer *) layer_get_data(layer);
	face_layer->back_layer = layer;
	face_layer->dash_long_bitmap = gbitmap_create_with_resource(RESOURCE_ID_DASH_LONG);
	face_layer->dash_short_bitmap = gbitmap_create_with_resource(RESOURCE_ID_DASH_SHORT);
	face_layer->hand_hour_bitmap = gbitmap_create_with_resource(RESOURCE_ID_HAND_HOUR);
	face_layer->hand_minute_bitmap = gbitmap_create_with_resource(RESOURCE_ID_HAND_MINUTE);

	face_layer->has_time = false;

	if (persist_exists(PersistDataKeyWatchfaceMode)) {
		face_layer->mode = persist_read_int(PersistDataKeyWatchfaceMode);
	}
	else {
		face_layer->mode = FaceLayerModeHandsWithAllDashes;
	}

	informer_add_listener(InformerEventMinuteTimer, face_layer, handle_minute_event);
	informer_add_listener(InformerEventSelectClick, face_layer, handle_select_event);

	return face_layer;
}


Layer *ui_face_layer_get_layer(struct FaceLayer *face_layer) {
	return face_layer->back_layer;
}


void ui_face_layer_destroy(struct FaceLayer *face_layer) {
	informer_remove_listener(InformerEventMinuteTimer, face_layer, handle_minute_event);
	informer_remove_listener(InformerEventSelectClick, face_layer, handle_select_event);

	layer_destroy(face_layer->back_layer);

	gbitmap_destroy(face_layer->dash_long_bitmap);
	gbitmap_destroy(face_layer->dash_short_bitmap);
	gbitmap_destroy(face_layer->hand_hour_bitmap);
	gbitmap_destroy(face_layer->hand_minute_bitmap);

	free(face_layer);
}
