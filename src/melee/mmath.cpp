/*
This file is part of "TW-Light"
					http://tw-light.appspot.com/
Copyright (C) 2001-2004  TimeWarp development team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "melee.h"
REGISTER_FILE
#include "mview.h"

#include "util/vector2.h"
double atan3(double y, double x)
{
	STACKTRACE;
	if (y == 0) {
		if (x >= 0) return 0;
		else return PI;
	}
	return atan2(y, x);
}


int sign (double n)
{
	if (n > 0) return 1;
	else if (n < 0) return -1;
	else return 0;
}


double trajectory_angle(double x1, double y1, double x2, double y2)
{
	STACKTRACE;
	double dx = min_delta(x2, x1, map_size.x);
	double dy = min_delta(y2, y1, map_size.y);
	double alpha = atan3(dy, dx);
	if (alpha < 0) alpha += PI2;
	return(alpha);
}


double trajectory_angle(Vector2 pos1, Vector2 pos2)
{
	STACKTRACE;
	double dx = min_delta(pos2.x, pos1.x, map_size.x);
	double dy = min_delta(pos2.y, pos1.y, map_size.y);
	double alpha = atan3(dy, dx);
	if (alpha < 0) alpha += PI2;
	return(alpha);
}


double intercept_angle (Vector2 pos, Vector2 vel, double v, Vector2 tpos, Vector2 tvel)
{
	Vector2 d, dv;
	double dist, time;
	d = min_delta(tpos, pos);
	dv = tvel - vel;
	dist = magnitude(d);
	time = dist / v;
	d += dv * time;
	return(atan(d));
}


double intercept_angle2(Vector2 pos, Vector2 vel, double v, Vector2 tpos, Vector2 tvel)
{
	STACKTRACE;
	Vector2 d, dv;
	double dist, time;
	d = min_delta(tpos, pos);
	dv = tvel - vel;
	dist = magnitude(d);
	time = dist / v / 2;
	d += dv * time;
	dist = magnitude(d);
	d += dv * (dist/v - time);
	return (atan(d));
}


double normalize(double value, double max)
{
	STACKTRACE;

	if (max <= 0) {
		tw_error ("normalize - bad \n%f\n %% %f", value, max);
		return normalize2(value, max);
	}
	if ((value > 999.0 * max) || (value < -999.0 * max)) {
		tw_error ("normalize - bad \n%f\n modulo %f", value, max);
		return normalize2(value, max);
	}
	while(value < 0.0)
		value += max;
	while(value >= max)
		value -= max;
	return(value);
}


double normalize2(double value, double max)
{
	STACKTRACE;
	//if (isnan(value) || isnan(max)) tw_error("normalize - not finite");
	value = fmod(value, max);
	if (value < 0) value += max;
	return(value);
}


Vector2 corner ( Vector2 pos, Vector2 size )
{
	pos -= space_center;
	while (pos.x < -map_size.x/2) pos.x += map_size.x;
	while (pos.x >  map_size.x/2) pos.x -= map_size.x;
	while (pos.y < -map_size.y/2) pos.y += map_size.x;
	while (pos.y >  map_size.y/2) pos.y -= map_size.x;
	pos -= size / 2;
	return pos * space_zoom + space_view_size / 2;
}


Vector2 corner ( Vector2 pos )
{
	return corner(pos, Vector2(0,0));
}


double min_delta(double from, double to, double max)
{
	STACKTRACE;
	double d1 = from - to;
	//  if (fabs(d1) * 2 < max) return d1;
	return normalize(d1 + max/2, max) - max/2;
}


Vector2 min_delta(Vector2 from, Vector2 to, Vector2 max)
{
	STACKTRACE;
	return Vector2 (
		min_delta(from.x,to.x,max.x),
		min_delta(from.y,to.y,max.y)
		);
}


double nearest_coord(double from, double to, double max)
{
	STACKTRACE;
	return(from - min_delta(from, to, max));
}


double distance_from(double x, double y, double to_x, double to_y)
{
	STACKTRACE;
	double dx = min_delta(x, to_x, map_size.x);
	double dy = min_delta(y, to_y, map_size.y);

	return(sqrt((dx * dx) + (dy * dy)));
}


double distance_from(Vector2 from, Vector2 to)
{
	STACKTRACE;
	double dx = min_delta(from.x, to.x, map_size.x);
	double dy = min_delta(from.y, to.y, map_size.y);

	return(sqrt((dx * dx) + (dy * dy)));
}


Vector2 normalize(Vector2 n, Vector2 max)
{
	STACKTRACE;
	return Vector2(normalize(n.x, max.x), normalize(n.y, max.y));
}


Vector2 normalize2(Vector2 n, Vector2 max)
{
	STACKTRACE;
	return Vector2(normalize2(n.x, max.x), normalize2(n.y, max.y));
}
