#ifndef __TW_ENDIAN_H__
#define __TW_ENDIAN_H__

int invert_ordering(int in) ;
int intel_ordering(int in) ;
int motorola_ordering(int in) ;

short invert_ordering_short(short in) ;
short intel_ordering_short(short in) ;
short motorola_ordering_short(short in) ;

#define little_endian_ordering        intel_ordering
#define little_endian_ordering_short  intel_ordering_short
#define big_endian_ordering           motorola_ordering
#define big_endian_ordering_short     motorola_ordering_short
#endif							 // __TW_ENDIAN_H__
