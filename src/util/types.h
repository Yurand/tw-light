#ifndef __TYPES_H__
#define __TYPES_H__

#if 0
#elif defined __GNUC__

//GNU C++

/*typedef uint64_t Uint64;
typedef uint32_t Uint32;
typedef uint16_t Uint16;
typedef uint8_t  Uint8;
typedef int64_t Sint64;
typedef int32_t Sint32;
typedef int16_t Sint16;
typedef int8_t  Sint8;*/
typedef unsigned long long int Uint64;
typedef unsigned int           Uint32;
typedef unsigned short int     Uint16;
typedef unsigned char          Uint8;

typedef signed long long int Sint64;
typedef signed int           Sint32;
typedef signed short int     Sint16;
typedef signed char          Sint8;

#elif defined _MSC_VER

//Micosoft Visual C++

typedef unsigned __int64 Uint64;
typedef unsigned __int32 Uint32;
typedef unsigned __int16 Uint16;
typedef unsigned __int8  Uint8;
typedef signed __int64 Sint64;
typedef signed __int32 Sint32;
typedef signed __int16 Sint16;
typedef signed __int8  Sint8;

#else

//unrecognized platform
//will guess type names
//and double-check in types.cpp

typedef unsigned long long int Uint64;
typedef unsigned int           Uint32;
typedef unsigned short int     Uint16;
typedef unsigned char          Uint8;

typedef signed long long int Sint64;
typedef signed int           Sint32;
typedef signed short int     Sint16;
typedef signed char          Sint8;
#endif
#endif							 // __TYPES_H__
