#include <pebble.h>
#include "date_layer.h"
#include "common.h"
#include "informer.h"


// types

struct DateLayer {
	Layer *back_layer;

	tm last_time;
};


// services

static void handle_date_tick(tm *time, TimeUnits changed_units) {
	informer_inform_with_object(InformerEventTimeTick, time);
}


// events

static void handle_date_tick_event(void *listener, void *object) {
	DateLayer *date_layer = listener;
	tm *time = object;

	date_layer->last_time = *time;
	layer_mark_dirty(date_layer->back_layer);
}


// core

static void date_layer_draw(Layer *layer, GContext *ctx) {
	DateLayer *date_layer = layer_get_data(layer);

}


DateLayer *date_layer_create(GRect rect) {
	Layer *layer = layer_create_with_data(rect, sizeof(DateLayer));
	layer_set_update_proc(layer, date_layer_draw);

	DateLayer *date_layer = layer_get_data(layer);
	date_layer->back_layer = layer;

	return date_layer;
}


Layer *date_layer_get_layer(DateLayer *date_layer) {
	return date_layer->back_layer;
}


void date_layer_got_focus(DateLayer *date_layer) {
	const time_t t = time(NULL);
	date_layer->last_time = *localtime(&t);
	layer_mark_dirty(date_layer->back_layer);

	informer_add_listener(InformerEventTimeTick, date_layer, handle_date_tick_event);

	tick_timer_service_subscribe(DAY_UNIT, handle_date_tick);
}


void date_layer_lost_focus(DateLayer *date_layer) {
	tick_timer_service_unsubscribe();

	informer_remove_listener(InformerEventTimeTick, date_layer, handle_date_tick_event);
}


void date_layer_destroy(DateLayer *date_layer) {
	layer_destroy(date_layer->back_layer);
}
