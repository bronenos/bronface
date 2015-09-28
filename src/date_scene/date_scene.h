#ifndef date_scene_H
#define date_scene_H

#include <pebble.h>


// types

struct DateScene;


// core

struct DateScene *date_scene_create();
Window *date_scene_get_window(struct DateScene *date_scene);
void date_scene_destroy(struct DateScene *date_scene);

#endif
