#include <pebble.h>
#include "main_window.h"
#include "face_layer.h"
#include "logic.h"


static void init() {
  logic_reset();

  logic()->window = ui_main_window_create();
  window_stack_push(logic()->window->window, true);
}


static void deinit() {
  ui_main_window_destroy(logic()->window);
}


int main(void) {
  init();
  app_event_loop();
  deinit();
  return 0;
}
