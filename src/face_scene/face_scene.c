#include "face_scene.h"
#include "face_layer.h"
#include "month_layer.h"
#include "informer.h"
#include "common.h"


// types

struct FaceScene {
	Window *window;
	FaceLayer *face_layer;
	MonthLayer *month_layer;
};


// window

static void handle_window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	const GRect window_bounds = layer_get_bounds(window_layer);

	FaceScene *face_scene = window_get_user_data(window);
	face_scene->face_layer = face_layer_create(window_bounds);
	face_scene->month_layer = NULL;
	layer_add_child(window_layer, face_layer_get_layer(face_scene->face_layer));

	face_layer_did_get_focus(face_scene->face_layer);
}


static void handle_window_unload(Window *window) {
	FaceScene *face_scene = window_get_user_data(window);

	if (face_scene->month_layer) {
		month_layer_did_lost_focus(face_scene->month_layer);
		month_layer_destroy(face_scene->month_layer);
	}

	if (face_scene->face_layer) {
		face_layer_did_lost_focus(face_scene->face_layer);
		face_layer_destroy(face_scene->face_layer);
	}
}


// events

static void handle_accel_event(void *listener, void *object) {
	FaceScene *face_scene = listener;
	AccelAxisType *axis = (AccelAxisType *) object;

	if (*axis == ACCEL_AXIS_Z) {
		if (face_scene->month_layer) {
			Layer *layer = month_layer_get_layer(face_scene->month_layer);
			layer_remove_from_parent(layer);

			face_scene->month_layer = NULL;

			month_layer_did_lost_focus(face_scene->month_layer);
			face_layer_did_get_focus(face_scene->face_layer);
		}
		else {
			Layer *window_layer = window_get_root_layer(face_scene->window);
			const GRect window_bounds = layer_get_bounds(window_layer);

			face_scene->month_layer = month_layer_create(window_bounds);

			Layer *layer = month_layer_get_layer(face_scene->month_layer);
			layer_add_child(window_layer, layer);

			face_layer_did_lost_focus(face_scene->face_layer);
			month_layer_did_get_focus(face_scene->month_layer);
		}
	}
}


// accel

static void handle_accel_tap(AccelAxisType axis, int32_t direction) {
	informer_inform_with_object(InformerEventAccel, &axis);
}


// timer

static void handle_timer(void *data) {
	AccelAxisType axis = ACCEL_AXIS_Z;
	handle_accel_event(data, &axis);
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

	informer_add_listener(InformerEventAccel, face_scene, handle_accel_event);

	accel_tap_service_subscribe(handle_accel_tap);

	app_timer_register(5500, handle_timer, face_scene);
	app_timer_register(15500, handle_timer, face_scene);

	return face_scene;
}


Window *face_scene_get_window(FaceScene *face_scene) {
	return face_scene->window;
}


void face_scene_destroy(FaceScene *face_scene) {
	accel_tap_service_unsubscribe();

	informer_remove_listener(InformerEventAccel, face_scene, handle_accel_event);

	window_destroy(face_scene->window);
	free(face_scene);
}
