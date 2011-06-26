#include "base.h"
//#include "types.h"
#include "errors.h"

COMPILE_TIME_ASSERT(sizeof(Uint8 ) == 1);
COMPILE_TIME_ASSERT(sizeof(Uint16) == 2);
COMPILE_TIME_ASSERT(sizeof(Uint32) == 4);
COMPILE_TIME_ASSERT(sizeof(Uint64) == 8);

COMPILE_TIME_ASSERT(sizeof(Sint8 ) == 1);
COMPILE_TIME_ASSERT(sizeof(Sint16) == 2);
COMPILE_TIME_ASSERT(sizeof(Sint32) == 4);
COMPILE_TIME_ASSERT(sizeof(Sint64) == 8);
