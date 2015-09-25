#include "main_window.h"
#include "face_layer.h"
#include "logic.h"


static void handle_window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	const GRect window_bounds = layer_get_bounds(window_layer);

	GRect rect = window_bounds;
	rect.origin.x += 10;
	rect.origin.y += 10;
	rect.size.w -= rect.origin.x * 2.0f;
	rect.size.h -= rect.origin.y * 2.0f;

	logic()->window->face_layer = ui_face_layer_create(rect);
	layer_add_child(window_layer, logic()->window->face_layer->back);
}


static void handle_window_unload(Window *window) {
	ui_face_layer_destroy(logic()->window->face_layer);
}


MainWindow *ui_main_window_create() {
	MainWindow *main_window = (MainWindow *) calloc(1, sizeof(MainWindow));
	main_window->window = window_create();

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
