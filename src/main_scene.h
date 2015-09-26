#ifndef main_scene_H
#define main_scene_H

#include <pebble.h>


// types
struct MainScene;


// core
struct MainScene *ui_main_scene_create();
Window *ui_main_scene_get_window(struct MainScene *main_scene);
void ui_main_scene_destroy(struct MainScene *main_scene);

#endif
