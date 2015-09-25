#ifndef FACE_LAYER_H
#define FACE_LAYER_H

#include <pebble.h>


typedef struct {
	Layer *back;
} FaceLayer;


FaceLayer *ui_face_layer_create(GRect rect);
void ui_face_layer_destroy(FaceLayer *layer);

#endif
