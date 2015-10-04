#include "face_scene.h"
#include "face_layer.h"
#include "date_layer.h"
#include "informer.h"
#include "common.h"


// types

struct FaceScene {
	Window *window;
	FaceLayer *face_layer;
	DateLayer *date_layer;

	struct {
		bool has_data;
		int16_t last_y;
		int16_t skip_samples;
		AppTimer *back_timer;
	} accel;
};


const int16_t kFaceSceneSwitchStrength	= 750;	


// timers

static void handle_back_timer(void *data) {
	FaceScene *face_scene = data;

	Layer *window_layer = window_get_root_layer(face_scene->window);
	Layer *layer = date_layer_get_layer(face_scene->date_layer);

	date_layer_lost_focus(face_scene->date_layer);
	layer_remove_from_parent(layer);
	face_layer_got_focus(face_scene->face_layer);

	face_scene->accel.back_timer = NULL;
}


// events

static void handle_accel_data_event(void *listener, void *object) {
	FaceScene *face_scene = listener;
	AccelData *data = object;

	if (face_scene->accel.skip_samples) {
		face_scene->accel.skip_samples--;
		face_scene->accel.last_y = data->y;
		return;
	}

	if (face_scene->accel.has_data) {
		if (abs(data->y - face_scene->accel.last_y) > kFaceSceneSwitchStrength) {
			Layer *window_layer = window_get_root_layer(face_scene->window);
			Layer *layer = date_layer_get_layer(face_scene->date_layer);

			if (face_scene->accel.back_timer) {
				app_timer_cancel(face_scene->accel.back_timer);
				face_scene->accel.back_timer = NULL;
			}

			if (layer_get_window(layer)) {
				date_layer_lost_focus(face_scene->date_layer);
				layer_remove_from_parent(layer);
				face_layer_got_focus(face_scene->face_layer);
			}
			else {
				face_layer_lost_focus(face_scene->face_layer);
				layer_add_child(window_layer, layer);
				date_layer_got_focus(face_scene->date_layer);

				face_scene->accel.back_timer = app_timer_register(7500, handle_back_timer, face_scene);
			}

			face_scene->accel.skip_samples = 10;
		}
	}

	face_scene->accel.last_y = data->y;
	face_scene->accel.has_data = true;
}


// services

static void handle_battery_state(BatteryChargeState battery) {
	informer_inform_with_object(InformerEventBattery, &battery);
}


static void handle_accel_tap(AccelAxisType axis, int32_t direction) {
	informer_inform_with_object(InformerEventAccelTap, &axis);
}


static void handle_accel_data(AccelData *data, uint32_t num_samples) {
	informer_inform_with_object(InformerEventAccelData, data);
}


// core

static void handle_window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	const GRect window_bounds = layer_get_bounds(window_layer);

	FaceScene *face_scene = window_get_user_data(window);
	face_scene->face_layer = face_layer_create(window_bounds);
	face_scene->date_layer = date_layer_create(window_bounds);

	layer_add_child(window_layer, face_layer_get_layer(face_scene->face_layer));
	face_layer_got_focus(face_scene->face_layer);
}


static void handle_window_unload(Window *window) {
	FaceScene *face_scene = window_get_user_data(window);

	if (face_scene->date_layer) {
		date_layer_lost_focus(face_scene->date_layer);
		date_layer_destroy(face_scene->date_layer);
		face_scene->date_layer = NULL;
	}

	if (face_scene->face_layer) {
		face_layer_lost_focus(face_scene->face_layer);
		face_layer_destroy(face_scene->face_layer);
		face_scene->face_layer = NULL;
	}
}


FaceScene *face_scene_create() {
	FaceScene *face_scene = malloc(sizeof(FaceScene));

	face_scene->window = window_create();
	memset(&face_scene->accel, 0, sizeof(face_scene->accel));

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
	informer_add_listener(InformerEventAccelData, face_scene, handle_accel_data_event);

	battery_state_service_subscribe(handle_battery_state);
	accel_tap_service_subscribe(handle_accel_tap);
	accel_data_service_subscribe(2, handle_accel_data);
}


void face_scene_lost_focus(FaceScene *face_scene) {
	if (face_scene->accel.back_timer) {
		app_timer_cancel(face_scene->accel.back_timer);
		face_scene->accel.back_timer = NULL;
	}

	accel_data_service_unsubscribe();
	accel_tap_service_unsubscribe();
	battery_state_service_unsubscribe();

	informer_remove_listener(InformerEventAccelData, face_scene, handle_accel_data_event);
}


void face_scene_destroy(FaceScene *face_scene) {
	window_destroy(face_scene->window);
	free(face_scene);
}
