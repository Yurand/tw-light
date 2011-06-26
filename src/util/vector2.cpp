#include <stdlib.h>
#include <math.h>
#include "base.h"
#include "vector2.h"
#include "random.h"
#include "round.h"
#include "errors.h"
//#include "../melee.h"

Vector2::Vector2(Vector2i v2i) : x(v2i.x), y(v2i.y)
{
	STACKTRACE;
}


Vector2i Vector2::round()
{
	STACKTRACE;
	return Vector2i(iround(x), iround(y));
}


Vector2i Vector2::truncate()
{
	STACKTRACE;
	return Vector2i(int(x), int(y));
}


double Vector2::length () const
{
	return sqrt(magnitude_sqr(*this));
}


double Vector2::angle() const
{
	if (*this == 0) return 0;
	double a = atan2(y, x);
	if (a < 0) a += 3.14159265358979 * 2;
	return a;
}


Vector2 Vector2::complex_divide(Vector2 other) const
{
	other.y *= -1;
	return complex_multiply(other) / magnitude_sqr(other);
}


Vector2 Vector2::rotate(double angle) const
{
	return complex_multiply(unit_vector(angle));
}


Vector2 tw_random ( Vector2 max )
{
	return Vector2(tw_random(max.x), tw_random(max.y));
}


Vector2 unit_vector ( double angle )
{
	Vector2 r;
	r.x = cos(angle);
	r.y = sin(angle);
	return r;
}


Vector2 unit_vector( Vector2 vec )
{
	STACKTRACE;
	double L;
	L = vec.length();

	if (L > 0)
		return vec / L;

	else
		return Vector2(1,0);
	// GEO: added default (1,0) vector returned if vec=0, ie, if the
	// result is undetermined.
}


Vector3D::Vector3D(double ax, double ay, double az)
{
	STACKTRACE;
	x = ax;
	y = ay;
	z = az;
}


double Vector3D::dot(Vector3D v)
{
	STACKTRACE;
	return x*v.x + y*v.y + z*v.z;
}


Vector3D Vector3D::cross(Vector3D b)
{
	STACKTRACE;
	return Vector3D(y*b.z - z*b.y, -x*b.z + z*b.x, x*b.y - y*b.x);
}


void Vector3D::normalize()
{
	STACKTRACE;
	double r;
	r = sqrt(x*x + y*y + z*z);
	x /= r;
	y /= r;
	z /= r;
}
