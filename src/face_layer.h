#ifndef FACE_LAYER_H
#define FACE_LAYER_H

#include <pebble.h>


typedef struct {
	Layer *back_layer;

	GBitmap *dash_long_bitmap;
	GBitmap *dash_short_bitmap;
	GBitmap *hand_hour_bitmap;
	GBitmap *hand_minute_bitmap;

	bool has_time;
	struct tm last_time;
} FaceLayer;


FaceLayer *ui_face_layer_create(GRect rect);
void ui_face_layer_destroy(FaceLayer *layer);

#endif
