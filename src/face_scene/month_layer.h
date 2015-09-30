#ifndef MONTH_LAYER_H
#define MONTH_LAYER_H

#include <pebble.h>

// types

struct MonthLayer;
typedef struct MonthLayer MonthLayer;


// core

MonthLayer *month_layer_create(GRect rect);
Layer *month_layer_get_layer(MonthLayer *month_layer);
void month_layer_did_get_focus(MonthLayer *month_layer);
void month_layer_did_lost_focus(MonthLayer *month_layer);
void month_layer_destroy(MonthLayer *month_layer);

#endif
