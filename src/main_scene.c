#include "main_scene.h"
#include "face_layer.h"
#include "informer.h"


// types
struct MainScene {
	Window *window;
	struct FaceLayer *face_layer;
};


// window handlers
static void handle_window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	const GRect window_bounds = layer_get_bounds(window_layer);

	struct MainScene *main_scene = (struct MainScene *) window_get_user_data(window);
	main_scene->face_layer = face_layer_create(window_bounds);
	layer_add_child(window_layer, face_layer_get_layer(main_scene->face_layer));
}


static void handle_window_unload(Window *window) {
	struct MainScene *main_scene = (struct MainScene *) window_get_user_data(window);
	face_layer_destroy(main_scene->face_layer);
}


// click handlers
static void handle_up_click(ClickRecognizerRef recognizer, void *context) {
	informer_inform_with_object(InformerEventUpClick, context);
}


static void handle_up_long_click(ClickRecognizerRef recognizer, void *context) {
	informer_inform_with_object(InformerEventUpLongClick, context);
}


static void handle_down_long_click(ClickRecognizerRef recognizer, void *context) {
	informer_inform_with_object(InformerEventDownLongClick, context);
}


static void handle_select_click(ClickRecognizerRef recognizer, void *context) {
	informer_inform_with_object(InformerEventSelectClick, context);
}


static void click_config_provider(void *context) {
	window_single_click_subscribe(BUTTON_ID_UP, handle_up_click);
	window_long_click_subscribe(BUTTON_ID_UP, 0, handle_up_long_click, NULL);
	window_long_click_subscribe(BUTTON_ID_DOWN, 0, handle_down_long_click, NULL);
	window_single_click_subscribe(BUTTON_ID_SELECT, handle_select_click);
}


// core
struct MainScene *main_scene_create() {
	struct MainScene *main_scene = (struct MainScene *) malloc(sizeof(struct MainScene));

	main_scene->window = window_create();
	window_set_user_data(main_scene->window, main_scene);
	window_set_click_config_provider(main_scene->window, click_config_provider);

	window_set_window_handlers(main_scene->window, (WindowHandlers) {
		.load = handle_window_load,
		.unload = handle_window_unload
	});

	return main_scene;
}


Window *main_scene_get_window(struct MainScene *main_scene) {
	return main_scene->window;
}


void main_scene_destroy(struct MainScene *main_scene) {
	window_destroy(main_scene->window);
	free(main_scene);
}
