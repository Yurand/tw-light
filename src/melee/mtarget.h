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

#ifndef __MTARGET_H__
#define __MTARGET_H__

#include "melee.h"
#include "mframe.h"

class Targets
{
	public:

		Targets();
		virtual ~Targets();

		int N;

		SpaceObject **item;

		virtual void add (SpaceObject *a);

		virtual void rem(int i);
		virtual void rem(SpaceObject *r);

		virtual void calculate();

		virtual void reset();

		int findindex(SpaceObject *o);
		bool isintargetlist(SpaceObject *o);
};

extern Targets *targets;
#endif							 // __MTARGET_H__
