#ifndef DATE_LAYER_H
#define DATE_LAYER_H

#include <pebble.h>

// types

struct DateLayer;
typedef struct DateLayer DateLayer;


// core

DateLayer *date_layer_create(GRect rect);
Layer *date_layer_get_layer(DateLayer *date_layer);
void date_layer_got_focus(DateLayer *date_layer);
void date_layer_lost_focus(DateLayer *date_layer);
void date_layer_destroy(DateLayer *date_layer);

#endif
