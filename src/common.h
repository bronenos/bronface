#ifndef COMMON_H
#define COMMON_H

// macros

#define log_verbose(fmt, args) APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, fmt, args)


// types

enum PersistDataKey {
	PersistDataKeyWatchfaceMode,
	PersistDataKeySecondsActive,
};

#endif