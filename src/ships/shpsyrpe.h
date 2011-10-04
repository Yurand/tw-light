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

#ifndef __SHPSYRPE_H__
#define __SHPSYRPE_H__

class SyreenPenetrator : public Ship
{
	public:
		double       weaponRange;
		double       weaponVelocity;
		int          weaponDamage;
		int          weaponArmour;

		double       specialRange;
		double       specialVelocity;
		int          specialDamage;
		int          specialFrames;

	public:
		SyreenPenetrator(Vector2 opos, double angle, ShipData *data, unsigned int code);

		virtual int activate_weapon();
		virtual int activate_special();
};

class CrewPod : public SpaceObject
{
	int frame_count;
	int frame_size;
	int frame_step;

	double velocity;
	int    life;
	int    lifetime;

	public:
		CrewPod(Vector2 opos, double oVelocity, int oLifeTime,
			Ship *oship, SpaceSprite *osprite, int ofcount, int ofsize);

		virtual void calculate();
		virtual int sameTeam(SpaceLocation *other);

		virtual void inflict_damage(SpaceObject *other);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
};
#endif
