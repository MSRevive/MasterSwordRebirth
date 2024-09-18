#ifndef STR_HELPER_H
#define STR_HELPER_H

#ifdef POSIX
char* strupr(char* start);
char* strlower(char* start);

#define _strupr strupr
#define _strlwr strlower
#define _snprintf snprintf
#define _vsnprintf(a, b, c, d) vsnprintf(a, b, c, d)
#define strnicmp strncasecmp
#define _strnicmp strncasecmp
#define _stricmp strcasecmp
#endif

#endif	// STR_HELPER_H