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
REGISTER_FILE

#include "shpmycpo.h"

MyconPodship::MyconPodship(Vector2 opos, double angle, ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)
{
	STACKTRACE;
	weaponRange    = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = tw_get_config_int("Weapon", "Damage", 0);
	weaponHome     = scale_turning(tw_get_config_float("Weapon", "Homing", 0));
	specialRepair  = tw_get_config_int("Special", "Repair", 0);
	plasma_shield  = 0;
}


int MyconPodship::activate_weapon()
{
	STACKTRACE;
	add(new MyconPlasma(Vector2(0.0, size.y),
		angle, weaponVelocity, weaponDamage, weaponRange, weaponHome, this,
		data->spriteWeapon, 64));
	return(TRUE);
}


int MyconPodship::activate_special()
{
	STACKTRACE;
	if (crew >= crew_max)
		return(FALSE);
	damage(this, 0, -4);
	return(TRUE);
}


SpaceSprite *MyconPlasma::spriteWeaponExplosion = NULL;
MyconPlasma::MyconPlasma(Vector2 opos, double oangle, double ov,
int odamage, double orange, double otrate, Ship *oship,
SpaceSprite *osprite, int ofcount) :
HomingMissile( oship, opos, oangle, ov, odamage, orange, 0, otrate, oship,
osprite, oship->target),
//  v(ov),
frame_count(ofcount),
max_damage(odamage)
{
	STACKTRACE;
	spriteWeaponExplosion = data->spriteWeaponExplosion;
	collide_flag_sameship |= bit(LAYER_SHIPS);
	collide_flag_sameteam |= bit(LAYER_SHIPS);
}


void MyconPlasma::calculate()
{
	STACKTRACE;
	HomingMissile::calculate();

	sprite_index = (int)((d / range) * (double)(frame_count - 1));
	damage_factor = max_damage - (int)((d / range) * (double)(max_damage));
}


void MyconPlasma::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (other == ship && dynamic_cast<MyconPodship*>(other)) {
		damage_factor *= 1.0 - ((MyconPodship*)ship)->plasma_shield;
	}
	SpaceObject::inflict_damage(other);
	//if (!other->isShot()) {
	if (other->isblockingweapons) {
		if (other->exists()) {
			add(new FixedAnimation(this, other,
				spriteWeaponExplosion, 0, 20, 50, DEPTH_EXPLOSIONS));
		} else {
			add(new Animation(this, other->normal_pos(),
				spriteWeaponExplosion, 0, 20, 50, DEPTH_EXPLOSIONS));
		}
		state = 0;
	}
}


int MyconPlasma::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	double total = normal + direct;

	if (total) {
		total += normal;
		total += direct;
		d += total / max_damage * range;
		if (d >= range) state = 0;
	}
	return 1;
}


REGISTER_SHIP(MyconPodship)
