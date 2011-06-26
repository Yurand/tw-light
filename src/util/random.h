#ifndef _TYPES_H
#   include "types.h"
#endif

#ifndef __RANDOM_H__
#define __RANDOM_H__

union split_int_64
{
	Uint64 whole;
	//this assumes little endian ordering but
	//since it's (so far) only used in x86 asm
	//code, that's not relevant
	struct blah
	{
		Uint32 low;
		Uint32 high;
	} s;
};

class RNG_lcg64a
{
	protected:
		split_int_64 s64;
	public:
		Uint8  raw8 () {return (Uint8) raw32();}
		Uint16 raw16() {return (Uint16)raw32();}
		Uint32 raw32();			 //does the actual work
		Uint64 raw64();

		Uint32 randi(Uint32 max);
		int    randi(int min, int max) {return min+randi (max-min);}
		double randf (double max) {return max * raw32() / 4294967296.0;}
		double randf (double min, double max) {return min+randf (max-min);}

		Uint64 get_state64() const {return s64.whole;}
		void set_state64 (Uint64 s) {s64.whole = s;}

		void fast_forward ( Uint64 how_far );
		void rewind ( Uint64 how_far ) {fast_forward(1 + ~how_far);}

		void seed(int s);
		void seed_more(int s);

		int serialize_state   (int max, unsigned char *destination) const;
		int deserialize_state (int max, const unsigned char *source) ;
};

typedef RNG_lcg64a RNG;
typedef RNG_lcg64a RNG_FS;
typedef RNG_lcg64a RNG_HQ;

extern RNG rng;

inline double tw_random(double a)
{return rng.randf(a);}
inline double tw_random(double min, double max)
{return rng.randf(min,max);}
inline int tw_random( int a )
{return rng.randi(a);}
inline Uint32 _tw_random()
{return rng.raw32();}
inline int tw_random()
{return rng.raw32()&0x7fffffff;}
#endif							 // __RANDOM_H__
