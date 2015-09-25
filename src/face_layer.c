#include <pebble.h>
#include "face_layer.h"
#include "logic.h"


static void ui_face_layer_update(Layer *layer, GContext *ctx) {
  const GRect layer_bounds = layer_get_bounds(layer);

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, layer_bounds, 0, GCornerNone);
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
