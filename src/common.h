#ifndef COMMON_H
#define COMMON_H

// macros

#define log_verbose(args...) APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, args)


// types

typedef enum {
	DeviceColorSilver,
	DeviceColorBlack,
	DeviceColorRose,
	DeviceColor_Count,
} DeviceColor;


typedef enum {
	PersistDataKeySecondsActive,
} PersistDataKey;


struct AccelParams {
	AccelAxisType axis;
	int32_t direction;
};
typedef struct AccelParams AccelParams;


typedef enum {
	AppMessageKeyURL,
	AppMessageKeyPlace,
	AppMessageKeyTemperature,
	AppMessageKeyDescription,
	AppMessageKeyPressure,
	AppMessageKeyHumidity,
} AppMessageKey;


struct WeatherInfo {
	char url[0xFF];
	char place[0x40];
	int16_t temperature;
	char description[0x20];
	int16_t pressure;
	int16_t humidity;
};
typedef struct WeatherInfo WeatherInfo;


// funcs

DeviceColor bk_get_device_color();
GPoint bk_second_point_for_rotation(GPoint center, int16_t length, int16_t angle);

#endif
