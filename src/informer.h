#ifndef INFORMER_H
#define INFORMER_H

// types

enum InformerEvent {
	InformerEventTimeTick,
	InformerEventUpClick,
	InformerEventUpLongClick,
	InformerEventDownLongClick,
	InformerEventDayTick,
};


typedef void(*InformerCallback)(void *listener, void *object);


// core

void informer_add_listener(enum InformerEvent event, void *listener, InformerCallback callback);
void informer_remove_listener(enum InformerEvent event, void *listener, InformerCallback callback);
void informer_inform_with_object(enum InformerEvent event, void *object);

#endif
