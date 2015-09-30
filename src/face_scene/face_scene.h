#ifndef face_scene_H
#define face_scene_H

#include <pebble.h>


// types

struct FaceScene;
typedef struct FaceScene FaceScene;


// core

FaceScene *face_scene_create();
Window *face_scene_get_window(FaceScene *face_scene);
void face_scene_got_focus(FaceScene *face_scene);
void face_scene_lost_focus(FaceScene *face_scene);
void face_scene_destroy(FaceScene *face_scene);

#endif
