#include <pebble.h>
#include "main_scene.h"
#include "face_layer.h"


static MainScene *init() {
  MainScene *main_scene = ui_main_scene_create();
  window_stack_push(main_scene->window, true);
  return main_scene;
}


static void deinit(MainScene *main_scene) {
  ui_main_scene_destroy(main_scene);
}


int main(void) {
  MainScene *main_scene = init();
  app_event_loop();
  deinit(main_scene);
  return 0;
}
