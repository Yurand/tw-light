#include <math.h>
#include <string.h>

#include "base.h"
#include "errors.h"
#include "types.h"
#include "random.h"

/*------------------------------
		RANDUM NUMBER GENERATOR
------------------------------*/

RNG rng;

inline Uint32 _rng_dist_32_flat ( Uint32 m, Uint32 r )
{
	#   if defined(_MSC_VER) && defined(__i386__) && !defined(NO_ASM)
	//figure out if this works on all MSVC versions
	//figure out how to detect x86 on MSVC
	_asm
	{
		mov eax, [r]
			mov edx, [m]
			mul edx
			mov [r], edx
	}
	return r;
	#   elif defined(__GNUC__) && defined(__i386__) && !defined(NO_ASM)
	asm ("mull %0" : "=d" (r) : "%a" (m), "0" (r) );
	return r;
	#   else					 //add gcc asm sometime
	return (Uint32)((r * (Uint64)m) >> 32);
	#   endif
}


enum
{
	MULTIPLIER = 1812433253,
	ADDITIVE   = 123456789
};
Uint32 RNG_lcg64a::randi(Uint32 max)
{
	STACKTRACE;
	return _rng_dist_32_flat(max, raw32());
}


Uint32 RNG_lcg64a::raw32()
{
	STACKTRACE;
	#   if defined(_MSC_VER) && defined(__i386__) && !defined(NO_ASM)
	split_int_64 i64 = s64;
	_asm { mov eax, [i64.s.high] }
	_asm { mov ebx, MULTIPLIER }
	_asm { imul eax, ebx }
	_asm { mov ecx, eax }
	_asm { mov eax, [i64.s.low] }
	_asm { mul ebx }
	_asm { add eax, ADDITIVE }
	_asm { adc ecx, edx}
	_asm { mov [i64.s.low], eax }
	_asm { mov [i64.s.high], ecx }
	s64 = i64;
	#   elif 0 && defined(__GNUC__) && !defined(NO_ASM)
	Uint32 _blah;
	asm (
		"imull %3, %5"  "\n\t"
		"mull  %3"      "\n\t"
		"addl  %6, %1"  "\n\t"
		"adcl  %0, %2"
		: "=d" (_blah),      "=a" (i64.s.low), "=b" (i64.s.high)
		: "0"  (MULTIPLIER), "1"  (i64.s.low),  "2" (i64.s.high),  "i" (ADDITIVE)
		);
	#   else
	s64.whole = (s64.whole * MULTIPLIER) + ADDITIVE;
	#   endif
	return s64.s.high;
}


Uint64 RNG_lcg64a::raw64()
{
	STACKTRACE;
	Uint64 bob = raw32();
	bob = (bob << 32) | raw32();
	return bob;
}


void RNG_lcg64a::fast_forward ( Uint64 how_far )
{

	if (how_far == 0) return;

	Uint64 tm = MULTIPLIER;
	Uint64 ta = ADDITIVE;

	while (1) {
		if (how_far & 1) s64.whole = s64.whole * tm + ta;
		how_far >>= 1;
		if (how_far == 0) return;
		ta = ta * tm + ta;
		tm = tm * tm;
	}
	return;
}


void RNG_lcg64a::seed( int s )
{
	STACKTRACE;
	s64.s.low = s;
	s64.s.high = 0;
	return;
}


void RNG_lcg64a::seed_more( int s )
{
	STACKTRACE;
	if (s64.s.high & 0x80000000) raw32();
	raw32();
	s64.s.low += ((s64.s.high >> 27) | (s64.s.high << 5)) ^ s;
	return;
}
