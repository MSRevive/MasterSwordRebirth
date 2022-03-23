#ifndef STR_HELPER_H
#define STR_HELPER_H

#ifdef POSIX
char *strupr( char *start );
char* strlower(char* start)

#define _strupr strupr
#define _strlwr strlower
#endif

#endif	// STR_HELPER_H