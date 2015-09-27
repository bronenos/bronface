#ifndef main_scene_H
#define main_scene_H

#include <pebble.h>


// types
struct MainScene;


// core
struct MainScene *main_scene_create();
Window *main_scene_get_window(struct MainScene *main_scene);
void main_scene_destroy(struct MainScene *main_scene);

#endif
