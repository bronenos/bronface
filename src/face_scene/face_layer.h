#ifndef FACE_LAYER_H
#define FACE_LAYER_H

#include <pebble.h>

// types

struct FaceLayer;
typedef struct FaceLayer FaceLayer;


// core

FaceLayer *face_layer_create(GRect rect);
Layer *face_layer_get_layer(FaceLayer *face_layer);
void face_layer_did_get_focus(FaceLayer *face_layer);
void face_layer_did_lost_focus(FaceLayer *face_layer);
void face_layer_destroy(FaceLayer *face_layer);

#endif
