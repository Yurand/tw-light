/* 
	These #defines control which algorithm is used for iround(),
iround_down(), and iround_up().  The asm algorithms are implemented on
x86 computers only, and should be very fast.  The binary algorithm assumes
that the the IEEE 64-bit floating point format is used for doubles, and that
the machine is little endian.  The C99 algorithm is probably as fast as the
asm algorithm, but only works on recent compilers.  The standard library
algorithm is slow, but should work everywhere.  Note that these algorithms
do not all produce identical results.  In particular, the BINARY version can
produce off-by-one results for large numbers, and the STDLIB version may
round in a subtly different way than the others.
*/

#include <math.h>
#include <float.h>

#include "base.h"
#include "round.h"

int iround     (double a) {return (int)floor(a+0.5);}
int iround_down(double a) {return (int)floor(a);}
int iround_up  (double a) {return (int)ceil(a);}
