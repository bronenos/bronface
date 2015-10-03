#include <pebble.h>
#include "face_scene/face_scene.h"
#include "informer.h"
#include "common.h"


// global

static FaceScene *g_face_scene;


// app messaging

static void app_message_inbox_received_callback(DictionaryIterator *iterator, void *context) {
	log_verbose("%s", __FUNCTION__);

	WeatherInfo *info = malloc(sizeof(WeatherInfo));

	Tuple *item = dict_read_first(iterator);
	while (item) {
		switch (item->key) {
		case AppMessageKeyPlace:
			strcpy(info->place, item->value->cstring);
			break;

		case AppMessageKeyTemperature:
			info->temperature = item->value->int32;
			break;
			
		case AppMessageKeyDescription:
			strcpy(info->description, item->value->cstring);
			break;
			
		case AppMessageKeyPressure:
			info->pressure = item->value->int32;
			break;
			
		case AppMessageKeyHumidity:
			info->humidity = item->value->int32;
			break;
		}

		item = dict_read_next(iterator);
	}

	informer_inform_with_object(InformerEventWeather, info);

	free(info);
}


static void app_message_inbox_dropped_callback(AppMessageResult reason, void *context) {
	log_verbose("%s", __FUNCTION__);
}


static void app_message_outbox_sent_callback(DictionaryIterator *iterator, void *context) {
	log_verbose("%s", __FUNCTION__);
}


static void app_message_outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
	log_verbose("%s", __FUNCTION__);
}


// core

static void init() {
	app_message_register_inbox_received(app_message_inbox_received_callback);
	app_message_register_inbox_dropped(app_message_inbox_dropped_callback);
	app_message_register_outbox_failed(app_message_outbox_failed_callback);
	app_message_register_outbox_sent(app_message_outbox_sent_callback);
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

	g_face_scene = face_scene_create();
	window_stack_push(face_scene_get_window(g_face_scene), false);
	face_scene_got_focus(g_face_scene);
}


static void deinit() {
	face_scene_lost_focus(g_face_scene);
	face_scene_destroy(g_face_scene);

	app_message_deregister_callbacks();
}


int main() {
	init();
	app_event_loop();
	deinit();
	return 0;
}
