#ifndef main_scene_H
#define main_scene_H

#include <pebble.h>
#include "face_layer.h"


typedef struct {
	Window *window;
	FaceLayer *face_layer;
} MainScene;


MainScene *ui_main_scene_create();
void ui_main_scene_destroy(MainScene *window);

#endif
