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

#include "shpdruma.h"

DruugeMauler::DruugeMauler(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;

	weaponRange         = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity      = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage        = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour        = tw_get_config_int("Weapon", "Armour", 0);
	weaponDriftVelocity = scale_velocity(tw_get_config_float("Weapon", "DriftVelocity", 0));
}


int DruugeMauler::activate_weapon()
{
	STACKTRACE;
	accelerate (this, angle + PI, weaponDriftVelocity / mass, MAX_SPEED);
	add(new DruugeMissile(
		Vector2(0.0, (get_size().y /*height()*/ / 2.0)), angle, weaponVelocity, weaponDamage,
		weaponDriftVelocity, weaponRange, weaponArmour, this,
		data->spriteWeapon));
	return(TRUE);
}


void DruugeMauler::calculate_fire_special()
{
	STACKTRACE;
	if ((fire_special) && (crew > 1) && (batt < batt_max) &&
	(special_recharge <= 0)) {

		batt += special_drain;
		if (batt > batt_max)
			batt = batt_max;
		crew--;

		special_recharge = special_rate;

		play_sound2(data->sampleSpecial[0]);
	}
}


DruugeMissile::DruugeMissile(Vector2 opos, double oangle, double ov,
int odamage, double weaponDriftVelocity, double orange, int oarmour,
Ship *oship, SpaceSprite *osprite) :
Missile(oship, opos, oangle, ov, odamage, orange, oarmour, oship, osprite),
kick(weaponDriftVelocity)
{
	STACKTRACE;
	explosionSprite     = data->spriteWeaponExplosion;
	explosionFrameCount = 5;
	explosionFrameSize  = 50;
}


void DruugeMissile::inflict_damage (SpaceObject *other)
{
	//	if (other->getID() == SPACE_PLANET) other->accelerate (this, angle, kick/25., GLOBAL_MAXSPEED);
	if (other->mass)
		other->accelerate (this, angle, kick / other->mass, MAX_SPEED);
	Missile::inflict_damage(other);
}


REGISTER_SHIP(DruugeMauler)
