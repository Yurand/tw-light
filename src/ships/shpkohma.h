/*
 *      shpkohma.h
 *
 *      Copyright 2008 Yura Siamashka <yurand2@gmail.com>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#ifndef __SHPKOHMA_H__
#define __SHPKOHMA_H__

#include "ship.h"

class KohrAhBlade;
class KohrAhMarauder : public Ship
{
	public:
		double       weaponRange;
		double       weaponVelocity;
		int          weaponDamage;
		int          weaponArmour;
		int          weaponFired;
		KohrAhBlade **weaponObject;
		bool         bladesPersist;

		double       specialVelocity;
		int          specialDamage;
		int          specialArmour;
		double       specialRange;

		KohrAhMarauder(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

		virtual int activate_weapon();
		virtual int activate_special();
		virtual void calculate();

		int numblades;
		int maxblades;
};

class KohrAhBlade : public AnimatedShot
{
	bool persist;
	int       passive;
	double    passiveRange;
	double    passiveVelocity;

	public:
		KohrAhBlade(Vector2 opos, double oangle, double ov, int odamage,
			double orange, int oarmour, Ship *oship,
			SpaceSprite *osprite, int ofcount, int ofsize, bool persist);

		virtual void calculate();
		virtual void animateExplosion();

		void disengage();
};

class KohrAhBladeDecay : public Animation
{
	public:
		KohrAhBladeDecay(SpaceLocation *creator, Vector2 opos, Vector2 ovel,
			SpaceSprite *osprite, int ofcount, int ofsize);
};

class KohrAhFRIED : public Shot
{
	int frame_count;

	public:
		KohrAhFRIED(double oangle, double ov, int odamage, double orange,
			int oarmour, Ship *oship, SpaceSprite *osprite, int ofcount);

		virtual void calculate();
		virtual void inflict_damage(SpaceObject *other);
};
#endif
