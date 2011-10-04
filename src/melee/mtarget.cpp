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

#include "mtarget.h"

Targets *targets;

Targets::Targets()
{
	STACKTRACE;
	N = 0;
	item = 0;
}


Targets::~Targets()
{
	reset();
}


void Targets::reset()
{
	STACKTRACE;
	if (item)
		free(item);
	item = 0;
	N = 0;
}


void Targets::add(SpaceObject *a)
{
	STACKTRACE;
	N += 1;
	item = (SpaceObject **) realloc(item, sizeof(SpaceObject *) * N);
	item[N - 1] = a;
	a->attributes |= ATTRIB_TARGET;
}


void Targets::rem(int i)
{
	STACKTRACE;
	-- N;
	item[i]->attributes &= ~ATTRIB_TARGET;
	item[i] = item[N];
}


void Targets::rem(SpaceObject *r)
{
	STACKTRACE;
	int i;
	for ( i = 0; i < N; ++i )
		if (item[i] == r)
			break;

	if (i == N)
		return;

	rem(i);
}


void Targets::calculate()
{
	STACKTRACE;
	int i;

	for (i = 0; i < N; i += 1) {
		if (!item[i]->exists()) {
			rem(i);
			-- i;
		}
	}
}


int Targets::findindex(SpaceObject *o)
{
	STACKTRACE;
	int i;

	for (i = 0; i < N; i += 1) {
		if (item[i] == o) {
			return i;
		}
	}

	return -1;
}


bool Targets::isintargetlist(SpaceObject *o)
{
	STACKTRACE;
	return findindex(o) >= 0;
}
