#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <pebble.h>
#include "face_layer.h"


typedef struct {
	Window *window;
	FaceLayer *face_layer;
} MainWindow;


MainWindow *ui_main_window_create();
void ui_main_window_destroy(MainWindow *window);

#endif
