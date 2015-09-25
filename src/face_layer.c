#include <pebble.h>
#include "face_layer.h"
#include "logic.h"


const int kFaceLayerHoursCount	= 12;


static GPath *ui_face_layer_build_hour_path(bool is_long) {
	GPoint *points = (GPoint *) malloc(4 * sizeof(GPoint));
	GPoint *p = points;

	int16_t offset, width, height;
	if (is_long) {
		offset = 6;
		width = 3;
		height = 16;
	}
	else {
		offset = 6;
		width = 3;
		height = 12;
	}

	*p++ = GPoint(-width, offset);
	*p++ = GPoint(width, offset);
	*p++ = GPoint(width, height);
	*p++ = GPoint(-width, height);

	GPathInfo path_info;
	path_info.num_points = 4;
	path_info.points = points;

  	return gpath_create(&path_info);
}


static void ui_face_layer_transform_hour_path(GPoint center_point, GPath *path, int16_t angle) {
	const int16_t path_dist_y = center_point.y - path->points[1].y;

	GPoint move_point;
	move_point.x = center_point.x + (path_dist_y * sin_lookup(angle) / TRIG_MAX_RATIO);
	move_point.y = center_point.y + (path_dist_y * -cos_lookup(angle) / TRIG_MAX_RATIO);

	gpath_move_to(path, move_point);
	gpath_rotate_to(path, angle);
}


static void ui_face_layer_update(Layer *layer, GContext *ctx) {
  const GRect layer_bounds = layer_get_bounds(layer);


  // fill with black color
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, layer_bounds, 0, GCornerNone);


  // draw hours
  graphics_context_set_fill_color(ctx, GColorWhite);

  GPoint center_point;
  center_point.x = layer_bounds.size.w / 2;
  center_point.y = layer_bounds.size.h / 2;

  for (int16_t i=0; i<kFaceLayerHoursCount; i++) {
  	const int16_t angle = TRIG_MAX_ANGLE * i / kFaceLayerHoursCount;
  	const bool is_third_hour = (i % 3 == 0);

  	GPath *path = ui_face_layer_build_hour_path(is_third_hour);
  	ui_face_layer_transform_hour_path(center_point, path, angle);
  	gpath_draw_filled(ctx, path);
  	gpath_destroy(path);
  	free(path->points);
  }
}


FaceLayer *ui_face_layer_create(GRect rect) {
	FaceLayer *face_layer = (FaceLayer *) calloc(1, sizeof(FaceLayer));

	face_layer->back = layer_create(rect);
	layer_set_update_proc(face_layer->back, ui_face_layer_update);

	return face_layer;
}


void ui_face_layer_destroy(FaceLayer *layer) {
	layer_destroy(layer->back);
	free(layer);
}
