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

#ifndef __SHPCHMAV_H__
#define __SHPCHMAV_H__

class ChmmrAvatar : public Ship
{
	public:
		double weaponRange;
		int    weaponDamage;

		double       specialForce;
		double       specialRange;

		double extraRange;
		int    extraDamage;
		int    extraFrames;
		int    extraRechargeRate;
		int    extraColor;
		int    extraArmour;

		bool   uninterrupted_fire;

	public:
		ChmmrAvatar(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

		virtual int activate_weapon();
		virtual int activate_special();
		virtual void calculate();
		virtual void materialize();
};

class ChmmrLaser : public Laser
{
	public:
		ChmmrLaser(double langle, double lrange, int ldamage, int lfcount,
			SpaceLocation *opos, Vector2 rel_pos);
};

class ChmmrBeam : public SpaceObject
{
	int frame;
	int frame_count;
	Ship *ship;
	SpaceObject *target;

	public:
		ChmmrBeam(Ship *oship, int oframes);

		virtual void calculate();
		virtual void animate(Frame *space);
};

class ChmmrZapSat : public SpaceObject
{
	int frame;

	double lRange;
	int    lDamage;
	int    lFrames;
	int    lRechargeRate;
	int    lRecharge;
	int    lColor;
	int    armour;

	public:
		ChmmrZapSat(double oangle, double orange, int odamage, int oframes,
			int orechargerate, int ocolor, int oarmour, Ship *oship,
			SpaceSprite *osprite);

		virtual void calculate();
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
		virtual int canCollide(SpaceLocation *other);
};
#endif
