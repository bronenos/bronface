#ifndef INFORMER_H
#define INFORMER_H

// types

enum InformerEvent {
	InformerEventTimeTick,
	InformerEventDateTick,
	InformerEventAccelTap,
	InformerEventAccelData,
	InformerEventBattery,
	InformerEventWeather,
};
typedef enum InformerEvent InformerEvent;


typedef void(*InformerCallback)(void *listener, void *object);


// core

void informer_add_listener(InformerEvent event, void *listener, InformerCallback callback);
void informer_remove_listener(InformerEvent event, void *listener, InformerCallback callback);
void informer_inform_with_object(InformerEvent event, void *object);

#endif
