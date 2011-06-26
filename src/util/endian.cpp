
#define PLATFORM_IS_ALLEGRO
#include "errors.h"
/*------------------------------
		Byte order (endianness)
------------------------------*/

#if 0
#elif defined PLATFORM_IS_ALLEGRO
#   include <allegro.h>
#   if defined ALLEGRO_LITTLE_ENDIAN
#       define TW_LITTLE_ENDIAN
#   elif defined ALLEGRO_BIG_ENDIAN
#       define TW_BIG_ENDIAN
#   else
#       error endianness not defined
#   endif
#elif defined PLATFORM_IS_SDL
#   if SDL_BYTEORDER == SDL_BIG_ENDIAN
#       define TW_BIG_ENDIAN
#   elif SDL_BYTEORDER == SDL_LITTLE_ENDIAN
#       define TW_LITTLE_ENDIAN
#   else
#       error endianness not defined
#   endif
#else
#   if  defined(__i386__) || defined(__ia64__) || defined(WIN32) || (defined(__alpha__) || defined(__alpha)) || defined(__arm__) || (defined(__mips__) && defined(__MIPSEL__)) || defined(__LITTLE_ENDIAN__)
#       define TW_LITTLE_ENDIAN
#   else
#       define TW_BIG_ENDIAN
#   endif
#endif

#if defined(TW_LITTLE_ENDIAN) && defined(TW_BIG_ENDIAN)
#   error Endian detection has failed (both big & little detected)
#elif !defined(TW_LITTLE_ENDIAN) && !defined(TW_BIG_ENDIAN)
#   error Endian detection has failed (neither big nor little detected)
#endif

int invert_ordering(int in)
{
	STACKTRACE;
	return (((in>>0)&0xff)<<24) + (((in>>8)&0xff)<<16) +
		(((in>>16)&0xff)<<8) + (((in>>24)&0xff)<<0);
}


short invert_ordering_short(short in)
{
	STACKTRACE;
	return (((in>>0)&0xff)<<8) + (((in>>8)&0xff)<<0) ;
}


#if defined TW_LITTLE_ENDIAN
int intel_ordering(int in)
{
	STACKTRACE;
	return in;
}


int motorola_ordering(int in)
{
	STACKTRACE;
	return invert_ordering(in);
}


short intel_ordering_short(short in)
{
	STACKTRACE;
	return in;
}


short motorola_ordering_short(short in)
{
	STACKTRACE;
	return invert_ordering_short(in);
}


#elif defined TW_BIG_ENDIAN
int intel_ordering(int in)
{
	STACKTRACE;
	return invert_ordering(in);
}


int motorola_ordering(int in)
{
	STACKTRACE;
	return in;
}


short intel_ordering_short(short in)
{
	STACKTRACE;
	return invert_ordering_short(in);
}


short motorola_ordering_short(short in)
{
	STACKTRACE;
	return in;
}


#else
#   error no endianness defined
#endif
