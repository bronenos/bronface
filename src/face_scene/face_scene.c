#include "face_scene.h"
#include "face_layer.h"
#include "informer.h"
#include "../date_scene/date_scene.h"
#include "common.h"


// types

struct FaceScene {
	Window *window;
	struct FaceLayer *face_layer;

	struct DateScene *date_scene;
};


// window

static void handle_window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	const GRect window_bounds = layer_get_bounds(window_layer);

	struct FaceScene *face_scene = (struct FaceScene *) window_get_user_data(window);
	face_scene->face_layer = face_layer_create(window_bounds);
	layer_add_child(window_layer, face_layer_get_layer(face_scene->face_layer));
}


static void handle_window_unload(Window *window) {
	struct FaceScene *face_scene = (struct FaceScene *) window_get_user_data(window);
	face_layer_destroy(face_scene->face_layer);
}


// events

static void handle_accel_event(void *listener, void *object) {
	struct FaceScene *face_scene = (struct FaceScene *) listener;
	AccelAxisType *axis = (AccelAxisType *) object;

	if (*axis == ACCEL_AXIS_Z) {
		face_scene->date_scene = date_scene_create();
		window_stack_push(date_scene_get_window(face_scene->date_scene), true);
	}
}


// accel

static void handle_accel_tap(AccelAxisType axis, int32_t direction) {
	informer_inform_with_object(InformerEventAccel, &axis);
}


// core

struct FaceScene *face_scene_create() {
	struct FaceScene *face_scene = (struct FaceScene *) malloc(sizeof(struct FaceScene));

	face_scene->window = window_create();
	window_set_user_data(face_scene->window, face_scene);

	window_set_window_handlers(face_scene->window, (WindowHandlers) {
		.load = handle_window_load,
		.unload = handle_window_unload
	});

	informer_add_listener(InformerEventAccel, face_scene, handle_accel_event);

	accel_tap_service_subscribe(handle_accel_tap);

	return face_scene;
}


Window *face_scene_get_window(struct FaceScene *face_scene) {
	return face_scene->window;
}


void face_scene_destroy(struct FaceScene *face_scene) {
	accel_tap_service_unsubscribe();

	informer_remove_listener(InformerEventAccel, face_scene, handle_accel_event);

	window_destroy(face_scene->window);
	free(face_scene);
}
