#include "date_scene.h"
#include "month_layer.h"
#include "informer.h"


// types

struct DateScene {
	Window *window;
	struct MonthLayer *month_layer;
};


// window

static void handle_window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	const GRect window_bounds = layer_get_bounds(window_layer);

	struct DateScene *date_scene = (struct DateScene *) window_get_user_data(window);
	date_scene->month_layer = month_layer_create(window_bounds);
	layer_add_child(window_layer, month_layer_get_layer(date_scene->month_layer));
}


static void handle_window_unload(Window *window) {
	struct DateScene *date_scene = (struct DateScene *) window_get_user_data(window);
	month_layer_destroy(date_scene->month_layer);
}


// core

struct DateScene *date_scene_create() {
	struct DateScene *date_scene = (struct DateScene *) malloc(sizeof(struct DateScene));

	date_scene->window = window_create();
	window_set_user_data(date_scene->window, date_scene);

	window_set_window_handlers(date_scene->window, (WindowHandlers) {
		.load = handle_window_load,
		.unload = handle_window_unload
	});

	return date_scene;
}


Window *date_scene_get_window(struct DateScene *date_scene) {
	return date_scene->window;
}


void date_scene_destroy(struct DateScene *date_scene) {
	free(date_scene);
}
