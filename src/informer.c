#include <pebble.h>
#include "informer.h"


// types

struct InformerItem {
	enum InformerEvent event;
	InformerCallback callback;
	void *listener;
};


struct Informer {
	struct InformerItem items[0xF];
	int16_t items_count;
} g_informer;


// core

void informer_add_listener(enum InformerEvent event, void *listener, InformerCallback callback) {
	g_informer.items[g_informer.items_count].event = event;
	g_informer.items[g_informer.items_count].callback = callback;
	g_informer.items[g_informer.items_count].listener = listener;
	g_informer.items_count++;
}


void informer_remove_listener(enum InformerEvent event, void *listener, InformerCallback callback) {
	for (int16_t i=0; i<g_informer.items_count; i++) {
		if (g_informer.items[i].event != event) {
			continue;
		}

		if (g_informer.items[i].listener != listener) {
			continue;
		}

		if (g_informer.items[i].callback != callback) {
			continue;
		}

		g_informer.items_count--;

		if (i < g_informer.items_count) {
			const int16_t number_to_move = g_informer.items_count - i;
			memmove(&g_informer.items[i], &g_informer.items[i + 1], number_to_move * sizeof(struct InformerItem));
		}
	}
}


void informer_inform_with_object(enum InformerEvent event, void *object) {
	for (int16_t i=0; i<g_informer.items_count; i++) {
		if (g_informer.items[i].event != event) {
			continue;
		}

		g_informer.items[i].callback(g_informer.items[i].listener, object);
	}
}

