#include <pebble.h>
#include "main_scene.h"


static struct MainScene *g_main_scene;


static void init() {
	g_main_scene = ui_main_scene_create();
	window_stack_push(ui_main_scene_get_window(g_main_scene), true);
}


static void deinit() {
	ui_main_scene_destroy(g_main_scene);
}


int main() {
	init();
	app_event_loop();
	deinit();
	return 0;
}
