/*
 *      shporzne.h
 *
 *      Copyright 2008 Unknown <yura@yura>
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

#ifndef __SHPORZNE_H__
#define __SHPORZNE_H__

#include "ship.h"

#define MAX_MARINES 8

class OrzNemesis;
class OrzMissile;
class OrzMarine;

class OrzNemesis : public Ship
{
	public:

		double absorption;		 //added for gob

		double       turret_turn_step;

		double       weaponRange;
		double       weaponVelocity;
		int          weaponDamage;
		int          weaponArmour;

		double       specialAccelRate;
		double       specialSpeedMax;
		int          specialHotspotRate;
		int          specialArmour;
		OrzMarine   *marine[MAX_MARINES];

		double       turretAngle;

		int          recoil;
		int          recoil_rate;
		int          recoil_range;
		double       turret_turn_rate;

		OrzNemesis(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

		virtual void calculate();
		virtual void animate(Frame *space);

		virtual void calculate_turn_left();
		virtual void calculate_turn_right();

		virtual int activate_weapon();
		virtual int activate_special();
};

class OrzMissile : public Missile
{
	public:
		OrzMissile(double oangle, double ov, int odamage, double orange,
			int oarmour, Ship *oship, SpaceSprite *osprite);
};

class OrzMarine : public SpaceObject
{
	double accel_rate;
	double speed_max;
	int    hotspot_rate;
	int    hotspot_frame;
	int    armour;
	Ship  *invading;
	int    returning;
	int    slot;
	int    damage_frame;

	OrzNemesis *orzship;

	public:
		OrzMarine(Vector2 opos, OrzNemesis *oship, double oAccelRate,
			double oSpeedMax, int oArmour, int oHotspotRate, int oSlot,
			SpaceSprite *osprite);

		void calculate();
		void animate(Frame *space);

		void inflict_damage(SpaceObject *other);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
};
#endif
