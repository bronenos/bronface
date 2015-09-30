#include <pebble.h>
#include "face_scene/face_scene.h"


static FaceScene *g_face_scene;


static void init() {
	g_face_scene = face_scene_create();
	window_stack_push(face_scene_get_window(g_face_scene), false);
	face_scene_got_focus(g_face_scene);
}


static void deinit() {
	face_scene_lost_focus(g_face_scene);
	face_scene_destroy(g_face_scene);
}


int main() {
	init();
	app_event_loop();
	deinit();
	return 0;
}
