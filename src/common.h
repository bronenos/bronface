#ifndef COMMON_H
#define COMMON_H

// macros

#define log_verbose(args...) APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, args)


// types

enum DeviceColor {
	DeviceColorSilver,
	DeviceColorBlack,
	DeviceColorRose,
	DeviceColor_Count,
};
typedef enum DeviceColor DeviceColor;

enum PersistDataKey {
	PersistDataKeyWatchfaceMode,
	PersistDataKeySecondsActive,
};
typedef enum PersistDataKey PersistDataKey;

struct AccelParams {
	AccelAxisType axis;
	int32_t direction;
};
typedef struct AccelParams AccelParams;


// funcs

DeviceColor bk_get_device_color();

#endif
