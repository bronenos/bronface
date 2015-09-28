#ifndef face_scene_H
#define face_scene_H

#include <pebble.h>


// types

struct FaceScene;


// core

struct FaceScene *face_scene_create();
Window *face_scene_get_window(struct FaceScene *face_scene);
void face_scene_destroy(struct FaceScene *face_scene);

#endif
