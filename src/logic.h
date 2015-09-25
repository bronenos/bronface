#ifndef LOGIC_H
#define LOGIC_H

#include <pebble.h>
#include "main_window.h"


typedef struct {
	MainWindow *window;
} Logic_t;


void logic_reset();
Logic_t *logic();

#endif
