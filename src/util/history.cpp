#include <stdlib.h>
#include <math.h>
#include "base.h"
#include "history.h"
#include "errors.h"

////////////////////////////////////////////////////////////////////////
//				Histograph stuff
////////////////////////////////////////////////////////////////////////

/*

X axi is "max"
Y axi is # of samples
This shows memory used, in bytes, not counting heap overhead.
Numbers are estimated.

 \  max   10      100     1000    10k     100k    1M
samples
10^1      0.08k   0.8k    8k      80k     800k    8M
10^2      0.32k   0.8k    8k      80k     800k    8M
10^3      0.56k   3.2k    8k      80k     800k    8M
...
10^6      1.36k   11.2k   88k     560k    3.2M    8M
10^9      2.16k   19.2k   168k    1360k   11.2M   88M
10^12     2.96k   27.2k   248k    2160k   19.2M   168M
10^15     3.76k   35.2k   328k    2960k   27.2M   248M
10^18     4.56k   43.2k   408k    3760k   35.2M   328M
*/

Histograph::Histograph(Uint16 max)
{
	STACKTRACE;
	num = 0;
	base = 0;
	this->max = max;
	element = new HISTOGRAPH_ELEMENT_TYPE[max];
	next = NULL;
}


Histograph::~Histograph()
{
	if (next)
		delete next;
	delete[] element;
}


void Histograph::add_element(double v)
{
	STACKTRACE;
	_add(v);
}


void Histograph::_add(double v)
{
	STACKTRACE;
	int i;
	if (num == max) {
		num -= next_ratio;
		double sum = 0;
		if (base <= next_ratio) {
			for (i = base - 1; i >= 0; i -= 1) {
				sum += element[i];
			}
			for (i = base - next_ratio + max; i < max; i += 1) {
				sum += element[i];
			}
		} else {
			for (i = base - next_ratio; i < base; i += 1) {
				sum += element[i];
			}
		}
		if (!next) next = new Histograph(max);
		next->_add(sum / next_ratio);
	}
	if (base == 0) base = max;
	base -= 1;
	element[base] = (HISTOGRAPH_ELEMENT_TYPE) v;
	num += 1;
	return;
}


double Histograph::_get (Sint64 back) const
{
	if (back < num) {
		int i = base + (int)back;
		if (i >= max) i -= max;
		return element[i];
	}
	if (!next) return 0;
	back -= num;
	//	double a = next->_get ( 1 + back/next_ratio );
	double b = next->_get ( back/next_ratio );
	return b;
}


double Histograph::get_element (Sint64 back) const
{
	return _get(back);
}


double Histograph::get_average (double back1, double back2) const
{
	if (back1==back2) return get_element((Sint64)back1);
	return get_integral(back1,back2)/(back2-back1);
}


double Histograph::get_integral (double back1, double back2) const
{
	if (back1 == back2) return 0;
	else if (back1 > back2) return -1 * get_integral(back2, back1);
	if (back1 < 0) back1 = 0;
	if (back2 < 0) back2 = 0;
	return _get_integral(back1, back2);
}


double Histograph::_get_integral (double back1, double back2) const
{
	int i;
	int bi1 = (int) back1;
	if (bi1 >= num) {
		if (!next) return 0;
		return next_ratio * next->_get_integral (
			(back1 - num) / next_ratio,
			(back2 - num) / next_ratio
			);
	}
	int bi2 = (int)ceil(back2) - 1;
	if (bi1 == bi2) {
		return _get(bi1) * (back2 - back1);
	}
	double sum;
	sum = _get(bi1) * (1.0 - (back1 - bi1));
	if (bi2 >= num) {
		for (i = bi1+1; i < num; i += 1) {
			sum += _get(i);
		}
		if (!next) return sum;
		return sum + next_ratio * next->_get_integral (
			0,
			(back2 - num) / next_ratio
			);
	}
	for (i = bi1+1; i < bi2; i += 1) {
		sum += _get(i);
	}
	return sum + _get(bi2) * (back2 - bi2);
}
