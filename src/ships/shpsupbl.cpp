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
#include "shpsupbl.h"

REGISTER_FILE

SupoxBlade::SupoxBlade(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;

	weaponRange    = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour   = tw_get_config_int("Weapon", "Armour", 0);
}


void SupoxBlade::calculate_thrust()
{
	STACKTRACE;

	if (thrust || ((fire_special) && (turn_left || turn_right || thrust))) {
		if ((fire_special) && (turn_left)) {
			accelerate_gravwhip(this, angle-PI/2, accel_rate * frame_time, speed_max);
		}
		if ((fire_special) && (turn_right)) {
			accelerate_gravwhip(this, angle+PI/2, accel_rate * frame_time, speed_max);
		}
		if ((fire_special) && (thrust)) {
			accelerate_gravwhip(this, angle+PI, accel_rate * frame_time, speed_max);
		}
		if ((!fire_special) && (thrust)) {
			accelerate_gravwhip(this, angle, accel_rate * frame_time, speed_max);
		}

	}
}


void SupoxBlade::calculate_turn_left()
{
	STACKTRACE;
	if (!fire_special)
		Ship::calculate_turn_left();
}


void SupoxBlade::calculate_turn_right()
{
	STACKTRACE;
	if (!fire_special)
		Ship::calculate_turn_right();
}


void SupoxBlade::calculate_hotspots()
{
	STACKTRACE;
	if (!fire_special)
		Ship::calculate_hotspots();
}


int SupoxBlade::activate_weapon()
{
	STACKTRACE;
	add(new Missile(this, Vector2(0.0, 0.5*get_size().y / 2.0),
		angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		this, data->spriteWeapon));
	return(TRUE);
}


void SupoxBlade::calculate_fire_special()
{
	STACKTRACE;
}


REGISTER_SHIP(SupoxBlade)
