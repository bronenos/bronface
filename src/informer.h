#ifndef INFORMER_H
#define INFORMER_H

enum InformerEvent {
	InformerEventMinuteTimer,
	InformerEventSelectClick,
};


typedef void(*InformerCallback)(void *listener, void *object);

void informer_add_listener(enum InformerEvent event, void *listener, InformerCallback callback);
void informer_remove_listener(enum InformerEvent event, void *listener, InformerCallback callback);
void informer_inform_with_object(enum InformerEvent event, void *object);

#endif
