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

#ifndef __ILWSP__
#define __ILWSP__

#include "ship.h"

#define ILWRATH_FIRE_ANIM_RATE           50
#define ILWRATH_MINE_FIRST_WHIRL_INDEX   13
// #define ILWRATH_MINE_LAST_WHIRL_INDEX    22
// it produced some artifacts
#define ILWRATH_MINE_LAST_WHIRL_INDEX    20
#define ILWRATH_MINE_SLOWDOWN_RATE       0.95
#define ILWRATH_SPECIAL_REL_X            0
#define ILWRATH_SPECIAL_REL_Y            (-size.y * 0.3)

class IlwrathSpiderMine : public SpaceObject
{
	int    step;
	int    life;
	int    inc;
	double randomness;
	int    stoptime;

	public:
		IlwrathSpiderMine( SpaceLocation *creator, double ox, double oy, double oangle, double v, int olife, double orandomness, int ostoptime, SpaceSprite* osprite );
		void calculate();
		void inflict_damage(SpaceObject *other);
};

class IlwrathStop : public SpaceLocation
{
	SpaceObject* victim;
	int          life;
	double       old_v;

	public:
		IlwrathStop( SpaceLocation* creator, SpaceObject* ovictim, int olife );
		virtual void calculate();
};
#endif
