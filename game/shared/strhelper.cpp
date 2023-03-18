#include <ctype.h>
#include <string.h>

#ifdef POSIX
char *strupr( char *start )
{
	unsigned char *str = (unsigned char*)start;
	while( *str )
	{
		if ( (unsigned char)(*str - 'a') <= ('z' - 'a') )
			*str -= 'a' - 'A';
		else if ( (unsigned char)*str >= 0x80 ) // non-ascii, fall back to CRT
			*str = toupper( *str );
		str++;
	}
	return start;
}

char* strlower(char* start)
{
	unsigned char* str = (unsigned char*)start;
	while (*str)
	{
		if ((unsigned char)(*str - 'A') <= ('Z' - 'A'))
			*str += 'a' - 'A';
		else if ((unsigned char)*str >= 0x80) // non-ascii, fall back to CRT
			*str = tolower(*str);
		str++;
	}
	return start;
}
#endif