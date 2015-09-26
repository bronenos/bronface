#ifndef FACE_LAYER_H
#define FACE_LAYER_H

#include <pebble.h>

struct FaceLayer;


struct FaceLayer *ui_face_layer_create(GRect rect);
Layer *ui_face_layer_get_layer(struct FaceLayer *face_layer);
void ui_face_layer_destroy(struct FaceLayer *face_layer);

#endif
