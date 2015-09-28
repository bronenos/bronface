#include <pebble.h>
#include "common.h"


enum DeviceColor bk_get_device_color() {
	switch (watch_info_get_color()) {
	case WATCH_INFO_COLOR_TIME_ROUND_SILVER_14:
	case WATCH_INFO_COLOR_TIME_ROUND_SILVER_20:
		return DeviceColorSilver;

	case WATCH_INFO_COLOR_TIME_ROUND_ROSE_GOLD_14:
		return DeviceColorRose;

	default:
		return DeviceColorBlack;
	}

	abort();
}
