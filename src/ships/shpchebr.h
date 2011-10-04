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

#ifndef __SHPCHEBR_H__
#define __SHPCHEBR_H__

class ChenjesuDOGI ;
class ChenjesuBroodhome : public Ship
{
	public:
		double      weaponVelocity, shardRange;
		double      shardDamage, shardArmour;
		double      shardRelativity;
		double      weaponDamage, weaponArmour;
		int         shardRotation;
		int         weaponFired;
		Shot        *weaponObject;

		double      specialVelocity;
		double      specialDamage;
		double      specialFuelSap;
		double      specialArmour;
		double      specialAccelRate;
		double      specialMass;
		double      specialAvoidanceAngle;
		double      specialAvoidanceFactor;
		int         specialNumDOGIs;

	public:
		ChenjesuBroodhome(Vector2 opos, double angle, ShipData *data, unsigned int code);
		virtual int activate_weapon();
		virtual int activate_special();
		virtual void calculate();
};

class ChenjesuShot : public Missile
{
	public:
		ChenjesuShot(Vector2 opos, double oangle, double ov, int odamage,
			int oarmour, SpaceLocation *creator, SpaceSprite *osprite);
		virtual void inflict_damage(SpaceObject *other);
		virtual void animateExplosion();
};

class ChenjesuDOGI : public AnimatedShot
{
	int     sap_factor;
	double  accel_rate;
	int     *num_dogis;

	public:
		ChenjesuDOGI(Vector2 opos, double oangle, double ov, int fuel_sap,
			int oarmour, double oaccel, double omass, Ship *oship,
			SpaceSprite *osprite, int *onum_dogis);
		double  avoidanceAngle;
		double  avoidanceFactor;
		virtual void calculate();
		virtual void inflict_damage(SpaceObject *other);
		virtual void death();
		virtual void ship_died();
		virtual void animateExplosion();
		virtual void soundExplosion();
};
#endif
