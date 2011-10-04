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

#ifndef __SHPKZEDR_H__
#define __SHPKZEDR_H__

class KzerZaDreadnought : public Ship
{
	public:
		double weaponRange;
		double weaponVelocity;
		int    weaponDamage;
		int    weaponArmour;

		int    specialFrames;
		int    specialLaserDamage;
		int    specialLaserColor;
		double specialLaserRange;
		int    specialLaserFrames;
		int    specialLaserDrain;
		double specialVelocity;
		double specialRange;
		int    specialArmour;

	public:
		KzerZaDreadnought(Vector2 opos, double angle, ShipData *data, unsigned int code);

		int activate_weapon();
		int activate_special();
};

class KzerZaMissile : public Missile
{
	public:
		KzerZaMissile(Vector2 opos, double oangle, double ov, int odamage,
			double orange, int oarmour, Ship *oship, SpaceSprite *osprite);
};

class KzerZaFighter : public Missile
{
	int    air_frames;
	int    max_air_frames;
	int    laser_damage;
	int    laser_color;
	double laser_range;
	int    laser_frames;
	int    batt;
	int    recharge_frames;

	public:
		KzerZaFighter(Vector2 opos, double oangle, double ov,
			double orange, int oarmour, int oair_frames,
			int olaser_damage, int olaser_color, double olaser_range,
			int olaser_frames, int odrain, Ship *oship, SpaceSprite *osprite);

		virtual void calculate();
		virtual void inflict_damage(SpaceObject *other);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
};
#endif
