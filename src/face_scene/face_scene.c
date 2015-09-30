#include "face_scene.h"
#include "face_layer.h"
#include "informer.h"
#include "common.h"


// types

struct FaceScene {
	Window *window;
	FaceLayer *face_layer;
};


// window

static void handle_window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	const GRect window_bounds = layer_get_bounds(window_layer);

	FaceScene *face_scene = window_get_user_data(window);

	face_scene->face_layer = face_layer_create(window_bounds);
	layer_add_child(window_layer, face_layer_get_layer(face_scene->face_layer));

	face_layer_got_focus(face_scene->face_layer);
}


static void handle_window_unload(Window *window) {
	FaceScene *face_scene = window_get_user_data(window);

	face_layer_lost_focus(face_scene->face_layer);
	face_layer_destroy(face_scene->face_layer);
}


// services

static void handle_battery_state(BatteryChargeState battery) {
	informer_inform_with_object(InformerEventBattery, &battery);
}


static void handle_accel_tap(AccelAxisType axis, int32_t direction) {
	informer_inform_with_object(InformerEventAccel, &axis);
}


// core

FaceScene *face_scene_create() {
	FaceScene *face_scene = malloc(sizeof(FaceScene));

	face_scene->window = window_create();
	window_set_user_data(face_scene->window, face_scene);

	window_set_window_handlers(face_scene->window, (WindowHandlers) {
		.load = handle_window_load,
		.unload = handle_window_unload
	});

	return face_scene;
}


Window *face_scene_get_window(FaceScene *face_scene) {
	return face_scene->window;
}


void face_scene_got_focus(FaceScene *face_scene) {
	battery_state_service_subscribe(handle_battery_state);
	accel_tap_service_subscribe(handle_accel_tap);
}


void face_scene_lost_focus(FaceScene *face_scene) {
	accel_tap_service_unsubscribe();
	battery_state_service_unsubscribe();
}


void face_scene_destroy(FaceScene *face_scene) {
	window_destroy(face_scene->window);
	free(face_scene);
}
