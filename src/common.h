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

enum PersistDataKey {
	PersistDataKeyWatchfaceMode,
	PersistDataKeySecondsActive,
};

struct AccelParams {
	AccelAxisType axis;
	int32_t direction;
};


// funcs

enum DeviceColor bk_get_device_color();

#endif
