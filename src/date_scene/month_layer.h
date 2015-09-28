#ifndef FACE_LAYER_H
#define FACE_LAYER_H

#include <pebble.h>

// types

struct MonthLayer;


// core

struct MonthLayer *month_layer_create(GRect rect);
Layer *month_layer_get_layer(struct MonthLayer *month_layer);
void month_layer_destroy(struct MonthLayer *month_layer);

#endif
