#ifndef STR_HELPER_H
#define STR_HELPER_H

#ifdef POSIX
char* strupr(char* start);
char* strlower(char* start);

// These need to be defined here otherwise we could fuck up SDL2's headers.
#define _strupr strupr
#define _strlwr strlower
#endif

#endif	// STR_HELPER_H