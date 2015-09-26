#include <pebble.h>
#include "main_scene.h"
#include "face_layer.h"


static struct MainScene *init() {
  struct MainScene *main_scene = ui_main_scene_create();
  window_stack_push(ui_main_scene_get_window(main_scene), true);
  return main_scene;
}


static void deinit(struct MainScene *main_scene) {
  ui_main_scene_destroy(main_scene);
}


int main(void) {
  struct MainScene *main_scene = init();
  app_event_loop();
  deinit(main_scene);
  return 0;
}
