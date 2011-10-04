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

#ifndef __SHPZFPST_H__
#define __SHPZFPST_H__

class ZoqFotPikStinger : public Ship
{
	public:
		double       weaponRange;
		double       weaponVelocity;
		int          weaponDamage;
		int          weaponArmour;

		int          specialDamage;

		ZoqFotPikStinger(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

		virtual int activate_weapon();
		virtual int activate_special();
};

class ZoqFotPikShot : public Shot
{
	int frame_count;

	public:
		ZoqFotPikShot(Vector2 opos, double oangle, double ov, int odamage,
			double orange, int oarmour, Ship *oship, SpaceSprite *osprite,
			int num_frames);

		virtual void calculate();
};

class ZoqFotPikTongue : public SpaceObject
{
	double dist;
	Ship  *ship;
	int    lick_factor;

	int frame;
	int frame_size;
	int frame_count;
	int frame_step;

	public:
		ZoqFotPikTongue(double odist, int odamage, Ship *oship,
			SpaceSprite *osprite, int ofcount, int ofsize);

		virtual void calculate();
		virtual void inflict_damage(SpaceObject *other);
};
#endif
