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


// clicks

static void handle_up_click_event(ClickRecognizerRef recognizer, void *context) {
	informer_inform_with_object(InformerEventUpClick, context);
}


static void handle_up_long_click_event(ClickRecognizerRef recognizer, void *context) {
	informer_inform_with_object(InformerEventUpLongClick, context);
}


static void handle_down_long_click_event(ClickRecognizerRef recognizer, void *context) {
	informer_inform_with_object(InformerEventDownLongClick, context);
}


static void handle_select_click_event(ClickRecognizerRef recognizer, void *context) {
	struct FaceScene *face_scene = (struct FaceScene *) context;
	face_scene->date_scene = date_scene_create();
	window_stack_push(date_scene_get_window(face_scene->date_scene), true);
}


static void click_config_provider(void *context) {
	window_single_click_subscribe(BUTTON_ID_UP, handle_up_click_event);
	window_long_click_subscribe(BUTTON_ID_UP, 0, handle_up_long_click_event, NULL);
	window_long_click_subscribe(BUTTON_ID_DOWN, 0, handle_down_long_click_event, NULL);
	window_single_click_subscribe(BUTTON_ID_SELECT, handle_select_click_event);
}


// core

struct FaceScene *face_scene_create() {
	struct FaceScene *face_scene = (struct FaceScene *) malloc(sizeof(struct FaceScene));

	face_scene->window = window_create();
	window_set_user_data(face_scene->window, face_scene);
	window_set_click_config_provider_with_context(face_scene->window, click_config_provider, face_scene);

	window_set_window_handlers(face_scene->window, (WindowHandlers) {
		.load = handle_window_load,
		.unload = handle_window_unload
	});

	return face_scene;
}


Window *face_scene_get_window(struct FaceScene *face_scene) {
	return face_scene->window;
}


void face_scene_destroy(struct FaceScene *face_scene) {
	window_destroy(face_scene->window);
	free(face_scene);
}
