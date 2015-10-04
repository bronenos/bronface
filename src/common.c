#include <pebble.h>
#include "common.h"


DeviceColor bk_get_device_color() {
	return DeviceColorBlack;
	
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


GPoint bk_second_point_for_rotation(GPoint center, int16_t length, int16_t angle) {
	GPoint point;
	point.x = center.x + (length * sin_lookup(angle) / TRIG_MAX_RATIO);
	point.y = center.y + (length * -cos_lookup(angle) / TRIG_MAX_RATIO);
	return point;
}
