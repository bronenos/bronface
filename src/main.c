#include <pebble.h>
#include "main_scene.h"


static struct MainScene *g_main_scene;


static void init() {
	g_main_scene = main_scene_create();
	window_stack_push(main_scene_get_window(g_main_scene), true);
}


static void deinit() {
	main_scene_destroy(g_main_scene);
}


int main() {
	init();
	app_event_loop();
	deinit();
	return 0;
}
