#include "main_scene.h"
#include "face_layer.h"


static void handle_window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	const GRect window_bounds = layer_get_bounds(window_layer);

	MainScene *main_scene = (MainScene *) window_get_user_data(window);
	main_scene->face_layer = ui_face_layer_create(window_bounds);
	layer_add_child(window_layer, main_scene->face_layer->back_layer);
}


static void handle_window_unload(Window *window) {
	MainScene *main_scene = (MainScene *) window_get_user_data(window);
	ui_face_layer_destroy(main_scene->face_layer);
}


MainScene *ui_main_scene_create() {
	MainScene *main_scene = (MainScene *) malloc(sizeof(MainScene));
	main_scene->window = window_create();
	window_set_user_data(main_scene->window, main_scene);

	window_set_window_handlers(main_scene->window, (WindowHandlers) {
		.load = handle_window_load,
		.unload = handle_window_unload
	});

	return main_scene;
}


void ui_main_scene_destroy(MainScene *main_scene) {
	window_destroy(main_scene->window);
	free(main_scene);
}
