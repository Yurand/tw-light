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

#ifndef __SHPMYCPO_H__
#define __SHPMYCPO_H__

class MyconPodship : public Ship
{
	public:
		double       weaponRange;
		double       weaponVelocity;
		int          weaponDamage;
		double       weaponHome;
		int          specialRepair;
		double       plasma_shield;

	public:
		MyconPodship(Vector2 opos, double angle, ShipData *data, unsigned int code);

		virtual int activate_weapon();
		virtual int activate_special();
};

class MyconPlasma : public HomingMissile
{
	//double v;

	static SpaceSprite *spriteWeaponExplosion;
	int frame_count;
	int max_damage;

	public:
		MyconPlasma(Vector2 opos, double oangle, double ov, int odamage,
			double orange, double otrate, Ship *oship, SpaceSprite *osprite, int ofcount);

		virtual void calculate();

		virtual void inflict_damage(SpaceObject *other);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
};
#endif
