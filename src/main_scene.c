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
	main_scene->face_layer = ui_face_layer_create(window_bounds);
	layer_add_child(window_layer, ui_face_layer_get_layer(main_scene->face_layer));
}


static void handle_window_unload(Window *window) {
	struct MainScene *main_scene = (struct MainScene *) window_get_user_data(window);
	ui_face_layer_destroy(main_scene->face_layer);
}


// tick handler
static void handle_minute_tick(struct tm *time, TimeUnits changed_units) {
	informer_inform_with_object(InformerEventMinuteTimer, time);
}


// click handlers
static void handle_select_click(ClickRecognizerRef recognizer, void *context) {
	informer_inform_with_object(InformerEventSelectClick, context);
}


static void click_config_provider(void *context) {
	window_single_click_subscribe(BUTTON_ID_SELECT, handle_select_click);
}


// core
struct MainScene *ui_main_scene_create() {
	struct MainScene *main_scene = (struct MainScene *) malloc(sizeof(struct MainScene));

	main_scene->window = window_create();
	window_set_user_data(main_scene->window, main_scene);
	window_set_click_config_provider(main_scene->window, click_config_provider);

	window_set_window_handlers(main_scene->window, (WindowHandlers) {
		.load = handle_window_load,
		.unload = handle_window_unload
	});

	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);

	return main_scene;
}


Window *ui_main_scene_get_window(struct MainScene *main_scene) {
	return main_scene->window;
}


void ui_main_scene_destroy(struct MainScene *main_scene) {
	tick_timer_service_unsubscribe();
	window_destroy(main_scene->window);
	free(main_scene);
}
