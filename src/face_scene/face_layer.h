#ifndef FACE_LAYER_H
#define FACE_LAYER_H

#include <pebble.h>

// types

struct FaceLayer;


// core

struct FaceLayer *face_layer_create(GRect rect);
Layer *face_layer_get_layer(struct FaceLayer *face_layer);
void face_layer_did_get_focus(struct FaceLayer *face_layer);
void face_layer_did_lost_focus(struct FaceLayer *face_layer);
void face_layer_destroy(struct FaceLayer *face_layer);

#endif
