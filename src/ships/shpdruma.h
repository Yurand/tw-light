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

#ifndef __SHPDRUMA_H__
#define __SHPDRUMA_H__

class DruugeMauler : public Ship
{
	public:
		double       weaponRange;
		double       weaponVelocity;
		int          weaponDamage;
		int          weaponArmour;
		double       weaponDriftVelocity;

	public:
		DruugeMauler(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

	protected:
		virtual int activate_weapon();
		virtual void calculate_fire_special();
};

class DruugeMissile : public Missile
{
	public:

		DruugeMissile(Vector2 opos, double oangle, double ov, int odamage,
			double weaponDriftVelocity, double orange, int oarmour, Ship *oship,
			SpaceSprite *osprite);

		virtual void inflict_damage (SpaceObject *other);
		double kick;
};
#endif
