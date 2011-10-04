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

#include "ship.h"
#include "allegro.h"
REGISTER_FILE

class StarBase : public Ship
{
	double       weaponRange;
	double       weaponVelocity;
	int          weaponDamage;
	int          weaponArmour;

	int    specialColor;
	double specialRange;
	int    specialFrames;
	int    specialDamage;
	int    HealRate;
	int    HealCost;
	int    HealColor;
	double    HealRange;

	public:
		StarBase(Vector2 opos, double angle, ShipData *data, unsigned int code);

	protected:
		virtual int activate_weapon();
		virtual int activate_special();
		virtual  int handle_damage(SpaceLocation *source, double normal, double direct = 0);
		virtual void calculate_gravity();
		virtual void calculate();
};

StarBase::StarBase(Vector2 opos, double angle, ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)
{
	STACKTRACE;

	weaponRange    = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour   = tw_get_config_int("Weapon", "Armour", 0);

	specialColor  = tw_get_config_int("Special", "Color", 0);
	specialRange  = scale_range(tw_get_config_float("Special", "Range", 0));
	specialFrames = tw_get_config_int("Special", "Frames", 0);
	specialDamage = tw_get_config_int("Special", "Damage", 0);

	HealRate = tw_get_config_int("Extra","HealRate",0);
	HealCost = tw_get_config_int("Extra","HealCost",0);
	HealColor  = tw_get_config_int("Extra", "HealColor", 0);
	HealRange  = scale_range(tw_get_config_float("Extra", "Range", 0));
}


int StarBase::activate_weapon()
{
	STACKTRACE;

	double a;
	if (target) {
		double r = distance(target);
		r = r / weaponVelocity;
		//		double x = min_delta( normal_x() , target->normal_x() -
		//				-target->get_vx() * r, X_MAX);
		//		double y = min_delta( normal_y() , target->normal_y() -
		//				-target->get_vy() * r, Y_MAX);
		Vector2 tmppos;
		tmppos = min_delta( normal_pos() , target->normal_pos() -
			-target->get_vel() * r, map_size);

		//		a = atan3(y, x) - PI;
		a = atan(tmppos) - PI;
	}
	else a = angle;

	game->add(new Missile(this, Vector2(-24.0, 14),
		a, weaponVelocity, weaponDamage, weaponRange, weaponArmour, this, data->spriteWeapon));
	game->add(new Missile(this, Vector2(24.0, 14),
		a, weaponVelocity, weaponDamage, weaponRange, weaponArmour, this, data->spriteWeapon));
	game->add(new Missile(this, Vector2(24.0, -14),
		a, weaponVelocity, weaponDamage, weaponRange, weaponArmour, this, data->spriteWeapon));
	return(TRUE);
}


int StarBase::activate_special()
{
	STACKTRACE;
	int fire = FALSE;
	SpaceObject *o;

	Query a;
	for (a.begin(this, bit(LAYER_SHIPS) + bit(LAYER_SHOTS) + bit(LAYER_SPECIAL) +
	bit(LAYER_CBODIES), specialRange); a.current; a.next()) {
		o = a.currento;
		if ( (!o->isInvisible()) && !o->sameTeam(this) && (o->collide_flag_anyone & bit(LAYER_LINES))) {
			SpaceLocation *l = new PointLaser(this, pallete_color[specialColor], 1,
				specialFrames, this, o, Vector2(0.0, 10.0));
			game->add(l);
			if (l->exists()) {
				fire = TRUE;
				l->set_depth(LAYER_EXPLOSIONS);
			}
		}
	}
	if (fire) play_sound((SAMPLE *)(melee[MELEE_BOOM + 0].dat));

	return(fire);
}


void StarBase::calculate()
{
	STACKTRACE;

	int fire = FALSE;
	Ship *o;

	Query a;
	for (a.begin(this, bit(LAYER_SHIPS), HealRange); a.current; a.next()) {
		o = (Ship *)a.currento;
		if ((!o->isInvisible()) && o->sameTeam(this)) {
			if ((random()%HealRate)!=0) continue;
			if (batt<HealCost) continue;
			if (o->crew==o->crew_max) continue;
			batt-=HealCost;
			SpaceLocation *l = new PointLaser(this, pallete_color[HealColor], -1,
				specialFrames, this, o, Vector2(0.0, 10.0));
			game->add(l);
			if (l->exists()) {
				fire = TRUE;
				l->set_depth(LAYER_EXPLOSIONS);
			}
		}
	}
	if (fire) play_sound((SAMPLE *)(melee[MELEE_BOOM + 0].dat));

	Ship::calculate();
}


int StarBase::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	batt -= normal;
	if (batt < 0) {
		normal = -batt;
		batt = 0;
	}
	else    normal = 0;
	return Ship::handle_damage(source, normal, direct);
}


void StarBase::calculate_gravity()
{
	STACKTRACE;
}


REGISTER_SHIP(StarBase)
