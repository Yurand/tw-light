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

#ifndef __ORBIT_H__
#define __ORBIT_H__

//#include <allegro.h>
//#include <stdio.h>

//#include "melee.h"
//#include "id.h"
//#include "scp.h"
//#include "frame.h"

//#include "melee/mgame.h"
//#include "melee/mview.h"
//#include "melee/mcbodies.h"
//#include "melee/manim.h"
//#include "melee/mship.h"

//#include "melee/mframe.h"

#define ORBIT_ID 0x26842116
#define SUN_ID 0x9f327223
#define MOON_ID 0x642344
#define COMET_ID 0x137853

extern void iMessage(char *, int);

class SpaceStation : public SpaceObject
{
	public:
		int Crew;
		SpaceStation(SpaceLocation *creator, Vector2 opos, SpaceSprite *oSprite);
		virtual void calculate();
		virtual int handle_damage(SpaceLocation *source, double normal, double direct = 0);
};

class OrbitHandler: public SpaceLocation
{
	public:
		SpaceLocation *sun, *plan;
		double Radius, Vel;
		Vector2 ipos;
		int Lock;
		OrbitHandler(SpaceLocation *creator, Vector2 lpos,
			double langle, SpaceLocation *p_sun, SpaceLocation *p_planet,
			double lrad, double lspeed, int iLock);

		virtual void calculate();
		virtual int canCollide(SpaceLocation *other);
};

class Sun : public SpaceObject
{
	public:
		double gravity_force;
		double gravity_mindist;
		double gravity_range;
		double gravity_power;
		double gravity_whip;
		Sun(Vector2 opos, SpaceSprite *sprite, int index);

		virtual void inflict_damage(SpaceObject *other);
		virtual void calculate();
		virtual int canCollide(SpaceLocation *other);
};
#endif							 // __ORBIT_H__
