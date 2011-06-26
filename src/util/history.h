#ifndef __HISTORY_H__
#define __HISTORY_H__

#include "types.h"

/*

Histograph "max"		Memory Usage
					Minimum		Typical	Maximum

10					128 bytes	1.6 k	4 kbytes
100					1 kbytes	15.2 k	44 kbytes
1,000				8 kbytes	120 k	400 kbytes
10,000				80 kytes	960 k	3.6 Mbytes
100,000				800 kbytes	7.2 M	35 Mbytes
1,000,000			8 Mbytes	48  M	328 Mbytes

That table assumes that floats are used for the elements.
If doubles are used instead, double all the memory usage numbers.
This tables also assumes that next_ratio is 2 (highly recommended).

Histograph should be able to handle at least 2**52 samples.
If "max" is at least 2,048 then it can probably handle a
full 2**63 samples.  The maximum memory consumption is an
estimate based upon about 2**60 samples.  Typical memory
consumption is based up 10**7.5 (~30 million) samples.  The
numbers listed are not precise.

If "max" is a power of 2 or slightly less, the heap manager may
be more efficient when allocating memory for a Histograph.
Inefficiencies in the heap manager might increase effective memory
consumption by a factor of 2.

*/

#define HISTOGRAPH_ELEMENT_TYPE float
//#define HISTOGRAPH_ELEMENT_TYPE double

class Histograph
{
	public:
		Histograph ( Uint16 max = 250 );
		~Histograph ();
		void add_element    ( double v );
		double get_element  ( Sint64 back ) const;
		double get_average  ( double back1, double back2 ) const;
		double get_integral ( double back1, double back2 ) const;
	private:
		void _add ( double v );
		double _get ( Sint64 back ) const;
		double _get_integral (double back1, double back2) const;

		Uint16 num;				 // 0 <= num < max
		Uint16 max;				 // 0 < max, recommended 8 <= max
		Uint16 base;			 // 0 <= base < max
		Uint16 _filler;			 //just for alignment
		enum { next_ratio = 2 };

		HISTOGRAPH_ELEMENT_TYPE *element;
		Histograph *next;
};
#endif							 // __HISTORY_H__
