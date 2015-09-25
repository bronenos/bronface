#include "main_window.h"
#include "face_layer.h"
#include "logic.h"


static void handle_window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	const GRect window_bounds = layer_get_bounds(window_layer);

	Logic *logic = (Logic *) window_get_user_data(window);
	logic->window->face_layer = ui_face_layer_create(window_bounds);
	layer_add_child(window_layer, logic->window->face_layer->back);
}


static void handle_window_unload(Window *window) {
	ui_face_layer_destroy(logic()->window->face_layer);
}


MainWindow *ui_main_window_create() {
	MainWindow *main_window = (MainWindow *) calloc(1, sizeof(MainWindow));
	main_window->window = window_create();
	window_set_user_data(main_window->window, logic());

	window_set_window_handlers(main_window->window, (WindowHandlers) {
		.load = handle_window_load,
		.unload = handle_window_unload
	});

	return main_window;
}


void ui_main_window_destroy(MainWindow *window) {
	window_destroy(window->window);
	free(window);
}
